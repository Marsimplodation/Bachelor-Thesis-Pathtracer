#include "window.h"
#include "accelerationStructures/lightFieldGrid.h"
#include "accelerationStructures/lightFieldGridSubBeams.h"
#include "common.h"
#include "primitives/object.h"
#include "scene/scene.h"
#include "shader/shader.h"
#include "tracer.h"
#include "accelerationStructures/bvh.h"
#include "types/camera.h"
#include "types/vector.h"

#include <SDL2/SDL.h>
#include <SDL_events.h>
#include <SDL_pixels.h>
#include <SDL_image.h>
#include <SDL_render.h>
#include <SDL_surface.h>
#include <SDL_video.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include <chrono>
#include <cmath>
#include <imgui.h>
#include <iostream>
#include <ratio>
#include <string>
#include <thread>
#include <vector>
namespace {
//INV_GAMMA = 1/2.2f
#define INV_GAMMA 0.4545f 
#define GAMMA 2.2f 
std::vector<visualInformation> fields = std::vector<visualInformation>();
auto tBegin = std::chrono::high_resolution_clock::now();
Object *selectedObject = 0x0;
char pixels[4096 * 2160 * 4];
bool quit, preview;
bool toneMapping = true;
bool editingColor = false;
} // namespace

bool &getToneMapping() {
    return toneMapping;
}

//credit to stackoverflow
//https://stackoverflow.com/questions/34255820/save-sdl-texture-to-file
void saveImage(const char* file_name, SDL_Renderer* renderer, SDL_Texture* texture) {
    SDL_Texture* target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, texture);
    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    SDL_Texture* targetTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
    SDL_SetRenderTarget(renderer, targetTexture);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch);
    IMG_SavePNG(surface, file_name);
    SDL_FreeSurface(surface);
    SDL_SetRenderTarget(renderer, target);
}


std::string objectNames(char flag, void* primitive, int n) {
    switch (flag) {
    case TRIANGLE:
        return "triangle" + std::to_string(n);
    default:
        return ((Object*)primitive)->name;
    }
}

void callReset() {
    reset();
    tBegin = std::chrono::high_resolution_clock::now();
}


u32 selectedMaterial = 0;
bool DisplayMaterial(Object * o) {
    bool change = false;
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1), "Materials");
    
    std::vector<const char*> mats(0);
    for(auto & mat : *getMaterials()) {
        if(!mat.name.empty())mats.push_back(mat.name.c_str()); 
    }
    if (ImGui::BeginCombo("Material", mats[selectedMaterial])) {
        for (int i = 0; i < o->materials.size(); i++) {
            int matIdx = o->materials[i];
            bool isSelected = (selectedMaterial == matIdx);
            if (ImGui::Selectable(mats[matIdx], isSelected)) {
                selectedMaterial = matIdx; 
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    Material * material = getMaterial(selectedMaterial);

    const char *items[] = {"Emissive", "Lambert", "Mirror", "Refract", "Edge", "None"};
    
    
    change |= ImGui::ColorEdit3("Albedo", (float *)&(material->pbr.albedo));
    change |= ImGui::DragFloat("Roughness", &(material->pbr.roughness), 0.01f, 0.0f, 1.0f);
    change |= ImGui::DragFloat("Emmision", &(material->pbr.emmision));
    change |= ImGui::DragFloat("Refractive Index 1", &(material->pbr.refractiveIdx1), 0.01f);
    change |= ImGui::DragFloat("Refractive Index 2", &(material->pbr.refractiveIdx2), 0.01f);

    // Store previous values
    float previousLambert = material->weights.lambert;
    float previousReflection = material->weights.reflection;
    float previousRefraction = material->weights.refraction;

    // Allow the user to change the weights
    change |= ImGui::DragFloat("Lambert Weight", &(material->weights.lambert), 0.01f, 0.0f, 1.0f);
    change |= ImGui::DragFloat("Reflection Weight", &(material->weights.reflection), 0.01f, 0.0f, 1.0f);
    change |= ImGui::DragFloat("Refraction Weight", &(material->weights.refraction), 0.01f, 0.0f, 1.0f);

    // Calculate the sum of the weights
    float sum = material->weights.lambert + material->weights.reflection + material->weights.refraction;

    // If the sum is not 1, normalize the weights
    if (sum != 1.0f) {
        material->weights.lambert /= sum;
        material->weights.reflection /= sum;
        material->weights.refraction /= sum;
    }
    
    if(material->weights.lambert >= 0.99f) {
        material->weights.lambert = 1.0f;
        material->weights.reflection = 0.0f;
        material->weights.refraction = 0.0f;
    }
    
    if(material->weights.refraction >= 0.99f) {
        material->weights.lambert = 0.0f;
        material->weights.reflection = 0.0f;
        material->weights.refraction = 1.0f;
    }
    
    if(material->weights.reflection >= 0.99f) {
        material->weights.lambert = 0.0f;
        material->weights.reflection = 1.0f;
        material->weights.refraction = 0.0f;
    }

    if (material->pbr.texture.data.size() > 0) {
        ImGui::Text("Texture Loaded");
        // Display texture or additional texture settings
    } else {
        ImGui::Text("No Texture Loaded");
    }

    return change;
}

void displayActiveObject() {
    ImGui::Begin("Object properties");
    if (!selectedObject) {
        ImGui::End();
        return;
    }
    bool change = false;
        Object *o = selectedObject;
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1), "%s", o->name.c_str());
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1), "Triangles: %d", o->endIdx - o->startIdx);
        change |= ImGui::Checkbox("Active", &o->active);
        change |= ImGui::DragInt2("indices", &(o->startIdx));
        change |= DisplayMaterial(o);

    if (change)
        callReset();
    ImGui::End();
};

void displayObjects() {
    ImGui::Begin("Objects");
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1), "Objects");
    ImGui::BeginChild("Scrolling");
    float windowWidth = ImGui::GetWindowWidth();
    int numP = getObjects().size();
    for (int n = 0; n < numP; n++) {
        auto &primitive = getObjects()[n];
        auto name = primitive.name;
        if (ImGui::Button(name.c_str(), ImVec2(windowWidth, 0))) {
            selectedObject = &primitive;
            selectedMaterial = primitive.materials[0];
        }
    }
    ImGui::EndChild();
    ImGui::End();
    displayActiveObject();
}

void displayMemory() {
    ImGui::Begin("Memory");
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1), "BVH %f GB", getMemoryBVH() / 1000000000.0f);
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1), "2Plane %f GB", getMemory2Plane() / 1000000000.0f);
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1), "SubBeams %f GB", getMemoryGridBeams() / 1000000000.0f);
    ImGui::End();
}

void displayCamera() {
    ImGui::Begin("Camera Setttings");
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1), "Camera");
    auto cam = getCamera();
    if (ImGui::DragFloat3("Camera forward", (float *)&(cam->forward))) {
        cameraSetForward(cam->forward);
        callReset();
    }
    if (ImGui::DragFloat3("Camera up", (float *)&(cam->up))) {
        cameraSetUp(cam->up);
        callReset();
    }
    if (ImGui::DragFloat3("Camera position", (float *)&(cam->origin))) {
        callReset();
    }
    if (ImGui::DragFloat("Camera Fov", &cam->fov)) {
        cameraSetFov(cam->fov);
        callReset();
    }
     if (ImGui::DragFloat("Depth of field", &cam->dof)) {
        //cameraSetFov(cam->fov);
        callReset();
    }

    if (ImGui::DragFloat("Camera focus", &cam->focus)) {
        //cameraSetFov(cam->fov);
        callReset();
    }
    
    if (ImGui::DragFloat("Camera yaw", &cam->yaw)) {
        rotate_camera({0,0}, 1.0f);
        callReset();
    }
    if (ImGui::DragFloat("Camera ptich", &cam->pitch)) {
        rotate_camera({0,0}, 1.0f);
        callReset();
    }
    if (ImGui::DragFloat("Camera Speed", &cam->speed)) {
    }
    if (ImGui::DragFloat("Camera sensitivy", &cam->sensitivity)) {
    }
    
    ImGui::End();
}

void displayIntersectSettings() {
    ImGui::Begin("Trace Setttings");
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1), "intersects: %lu", getIntersectionCount());
    const char *items[] = {"ALL", "BVH", "2Plane", "SubBeams"};
    if(ImGui::Checkbox("NEE", &getNEE())) {
        callReset();
    }
    if (ImGui::BeginCombo("intersect mode", items[getIntersectMode()])) {
        for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
            bool isSelected = (getIntersectMode() == i);
            if (ImGui::Selectable(items[i], isSelected)) {
                setIntersectMode(i);
                callReset();
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::End();

}

void displayMenu(SDL_Renderer *renderer, SDL_Texture *texture) {
    ImGui::Begin("Menu");
    float windowWidth = ImGui::GetWindowWidth();
    if (ImGui::Button("Quit", ImVec2(windowWidth, 0))) {
        destroyTracer();
        quit = true;
    }
    const char *items[] = {"ALL.png", "BVH.png", "Subgrids.png", "Subbeams.png"};
    auto file_name = std::string("./render_") +
        (getDebugView() ? getDebugShowTris() ? "heatmap_tris_" : "heatmap_as_": "")  + items[getIntersectMode()];
    if(ImGui::Button("Save")) saveImage(file_name.c_str(), renderer, texture);
    ImGui::TextColored(ImVec4(0.8, 0.8, 0.8, 1), "Window Settings");
    ImGui::Checkbox("Test Size", &preview);
    ImGui::Checkbox("Tonemapping", &toneMapping);
    bool change = false;
    change |= ImGui::DragInt("Max samples", &getMaxSampleCount());
    change |= ImGui::Checkbox("Debug", &getDebugView());
   
    if(getDebugView()){
        change |= ImGui::Checkbox("Triangles", &getDebugShowTris());
        change |= ImGui::DragInt("Debug Visual", &getDebugScale());
    }

    if(change) callReset();

    ImGui::End();

}

//interactive ui
ImVec2 rendering_pos;
ImVec2 rendering_size;
Vector2 keyboard = {0,0};
Vector2 mouse = {0,0};

void createWindow(bool testing) {
    SDL_Window *window;
    quit = false;
    preview = testing;
    SDL_Init(SDL_INIT_VIDEO);
    int WIDTH = getWindowSize().x;
    int HEIGHT = getWindowSize().y;
    window = SDL_CreateWindow("Pixel Drawing", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, WIDTH + 200, HEIGHT,
                              SDL_INIT_VIDEO);
    if (window == nullptr) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    // Create a renderer
    SDL_Renderer *renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "Failed to create renderer: " << SDL_GetError()
                  << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO &io{ImGui::GetIO()};
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    // Set the draw color (e.g., red)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    // Main loop
    SDL_Event e;
    ImVec2 previewSize(0, 0);
    double elapsed_time_ms = 0.0;

    // Variables to track delta time
    Uint32 current_time = 0;
    Uint32 last_time = 0;
    float delta_time = 0.0f;
    bool controling = false;
    bool controling_m = false;
    float lastMouseX = 0.0f;
    float lastMouseY = 0.0f;
    while (!quit) {
        current_time = SDL_GetTicks(); // Get the number of milliseconds since SDL_Init was called
        delta_time = (current_time - last_time) / 1000.0f; // Convert to seconds
        last_time = current_time;
        move_camera(keyboard, delta_time);
        if(controling && controling_m){
            controling_m = false;
            std::thread t(rotate_camera, mouse, delta_time);
            t.join();}
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (e.type == SDL_QUIT) {
                quit = true;
            }

            if(e.type == SDL_KEYDOWN)  {
                switch (e.key.keysym.sym) {
                    case SDLK_w: keyboard[0] = 1.0f; break;
                    case SDLK_s: keyboard[0] = -1.0f; break;
                    case SDLK_a: keyboard[1] = -1.0f; break;
                    case SDLK_d: keyboard[1] = 1.0f; break;
                    default: break;
                }

            }
            if(e.type == SDL_KEYUP)  {
                switch (e.key.keysym.sym) {
                    case SDLK_w: keyboard[0] = 0.0f; break;
                    case SDLK_s: keyboard[0] = 0.0f; break;
                    case SDLK_a: keyboard[1] = 0.0f; break;
                    case SDLK_d: keyboard[1] = 0.0f; break;
                    default: break;
                }
            }
            // Mouse button down (equivalent to SAPP_EVENTTYPE_MOUSE_DOWN)
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                // Check if the mouse is inside the rendering area
                if(!ImGui::IsMouseHoveringRect(rendering_pos, rendering_size, false)) continue;;

                // Hide and lock the mouse
                SDL_ShowCursor(SDL_DISABLE);
                SDL_SetRelativeMouseMode(SDL_TRUE); // Lock the mouse
                controling = true;
                getPrimaryOnly() = true;
                callReset();
            }

            // Mouse button up (equivalent to SAPP_EVENTTYPE_MOUSE_UP)
            if (e.type == SDL_MOUSEBUTTONUP) {
                controling = false;
                // Show and unlock the mouse
                SDL_ShowCursor(SDL_ENABLE);
                SDL_SetRelativeMouseMode(SDL_FALSE); // Unlock the mouse
                if(!ImGui::IsMouseHoveringRect(rendering_pos, rendering_size, false)) continue;;
                mouse[0] = 0.0f;
                mouse[1] = 0.0f;

                getPrimaryOnly() = false;
                callReset();
            }

            // Mouse movement (equivalent to SAPP_EVENTTYPE_MOUSE_MOVE)
            if (e.type == SDL_MOUSEMOTION) {
                controling_m = true;
                if(!ImGui::IsMouseHoveringRect(rendering_pos, rendering_size, false)) continue;;
                    mouse[0] = -e.motion.xrel;  // Reverse X-axis
                    mouse[1] = e.motion.yrel;   // Y-axis (SDL Y-axis goes top to bottom)
                    lastMouseX = e.motion.x;
                    lastMouseY = e.motion.y;

                // Store mouse delta values (Note: SDL gives relative motion when in relative mode)
                if (controling) {
                    float frame_duration = 0.016f; // Replace this with actual frame duration
                    int w; int h;
                    SDL_GetWindowSize(window, &w, &h);
                    SDL_WarpMouseInWindow(window, w / 2, h / 2);
                }
            }
        }
        
        // Calculate delta time

        SDL_SetRenderDrawColor(renderer, 0, 0, 0,
                               255); // Clear the texture to black
        SDL_RenderClear(renderer);

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        // show rendering time
        if(!getfinishedRendering()) {
            auto tNow = std::chrono::high_resolution_clock::now();
            elapsed_time_ms = std::chrono::duration<double, std::milli>(tNow - tBegin).count();
        }
        int seconds = (elapsed_time_ms) / 1000;
        int minutes = seconds / 60;
        seconds %= 60;
        int hours = minutes / 60;
        minutes %= 60;
        // char buffer[64];
        // sprintf(buffer, "Rendering Time: %02d:%02d:%02d", hours, minutes,
        // seconds);
        ImGuiWindowFlags overlayFlags =
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

        bool showOverlay = true;
        // ImGui::Begin("Overlay", &showOverlay,overlayFlags);
        ImGui::Begin("Information", &showOverlay);
        ImGui::TextColored(ImVec4(0.8, 0.8, 0.8, 1),
                           "Rendering Time: %02d:%02d:%02d", hours, minutes,
                           seconds);
        ImGui::End();
        displayObjects();
        displayCamera();
        displayMemory();
        displayIntersectSettings();

        ImGui::Begin("Rendering", nullptr);
        if (preview) {
            ImGuiWindowFlags previewFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_Popup;
            ImVec2 renderingSize = ImGui::GetWindowSize();
            ImGui::End();
            ImGui::Begin("Render", nullptr);
            ImGui::SetWindowSize("Render", ImVec2(1280, 720));
        }
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImVec2 size = ImGui::GetWindowSize();
        size = ImVec2(size.x, size.y);
        if ((int)size.x != WIDTH || (int)size.y != HEIGHT) {
            if(!testing) setWindowSize(size.x, size.y);
            WIDTH = getWindowSize().x;
            HEIGHT = getWindowSize().y;
            tBegin = std::chrono::high_resolution_clock::now();
        }
        rendering_pos = ImGui::GetWindowPos();
        rendering_size = ImGui::GetWindowSize();
        rendering_size = {rendering_size.x + rendering_pos.x, rendering_size.y + rendering_pos.y};
        // Draw pixels
        
        //simple toneMapping
        Vector3 oneVector = {1,1,1};
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                auto c = tracerGetPixel((int)(x),
                                        (int)(y));
                if (toneMapping) {
                    c = c / ( oneVector + c); // simple tonemapping
                    // gamma correct
                    c[0] = std::powf(c[0], INV_GAMMA);
                    c[1] = std::powf(c[1], INV_GAMMA);
                    c[2] = std::powf(c[2], INV_GAMMA);
                }
                c = clampToOne(c); // Assuming clampToOne ensures c is in [0,1]
                // Convert to 0-255 range
                unsigned char r = (unsigned char)(c.x * 255);
                unsigned char g = (unsigned char)(c.y * 255);
                unsigned char b = (unsigned char)(c.z * 255);
                
                // Assign to pixels buffer
                pixels[(y * WIDTH + x) * 4 + 0] = 0;
                pixels[(y * WIDTH + x) * 4 + 1] = b;
                pixels[(y * WIDTH + x) * 4 + 2] = g;
                pixels[(y * WIDTH + x) * 4 + 3] = r;
            }
        }
        SDL_Texture *texture =
            SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                              SDL_TEXTUREACCESS_STATIC, WIDTH, HEIGHT);
        SDL_UpdateTexture(texture, NULL, pixels, WIDTH * sizeof(char) * 4);
        ImGui::Image((void *)texture, ImVec2(WIDTH, HEIGHT));
        ImGui::End();
        displayMenu(renderer, texture);

        if(testing && getfinishedRendering()) {
            const char *items[] = {"ALL.png", "BVH.png", "Subgrids.png", "Subbeams.png"};
            auto file_name = std::string("./render/render_") +
            (getDebugView() ? getDebugShowTris() ? "heatmap_tris_" : "heatmap_as_": "")  + items[getIntersectMode()];
            saveImage(file_name.c_str(), renderer, texture);
            quit = true;
        }

        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_DestroyTexture(texture);
        // Update the window
        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
