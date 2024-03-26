#include "window.h"
#include "../primitives/cube.h"
#include "../primitives/object.h"
#include "../primitives/plane.h"
#include "../primitives/sphere.h"
#include "../primitives/triangle.h"
#include "../scene/scene.h"
#include "../shader/shader.h"
#include "../tracer.h"
#include "../types/camera.h"

#include <SDL2/SDL.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include <chrono>
#include <imgui.h>
#include <iostream>
#include <ratio>
#include <string>
#include <vector>
namespace {
std::vector<visualInformation> fields = std::vector<visualInformation>();
auto tBegin = std::chrono::high_resolution_clock::now();
std::string activeObject = "";
void *activeObjectPtr = 0x0;
char activeObjectFlag = 0x0;
char pixels[4096 * 2160 * 4];
} // namespace

std::string objectNames(char flag) {
    switch (flag) {
    case CUBE:
        return "cube";
    case PLANE:
        return "plane";
    case TRIANGLE:
        return "triangle";
    case SPHERE:
        return "sphere";
    case OBJECT:
        return "object";
    default:
        return "cube";
    }
}

void callReset() {
    reset();
    tBegin = std::chrono::high_resolution_clock::now();
}

bool DisplayShaderInfo(SimpleShaderInfo *info) {
    bool change = false;
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1), "Shader");
    const char *items[] = {"Emissive", "Lambert", "Mirror", "Refract", "None"};

    if (ImGui::BeginCombo("Material Type", items[info->shaderFlag])) {
        for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
            bool isSelected = (info->shaderFlag == i);
            if (ImGui::Selectable(items[i], isSelected)) {
                info->shaderFlag = i;
                change = true;
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    change |= ImGui::ColorEdit3("Color", (float *)&(info->color));
    change |= ImGui::DragFloat("Intensity", &(info->intensity));
    change |= ImGui::DragFloat("refractive index 1", &(info->refractiveIdx1));
    change |= ImGui::DragFloat("refractive index 2", &(info->refractiveIdx2));
    return change;
}

void displayActiveObject() {
    ImGui::Begin("Object properties");
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1), "%s", activeObject.c_str());
    if (activeObject == "") {
        ImGui::End();
        return;
    }
    bool change = false;
    if (activeObjectFlag == SPHERE) {
        Sphere *s = (Sphere *)activeObjectPtr;
        change |= ImGui::DragFloat3("Center", (float *)&(s->center));
        change |= ImGui::DragFloat("Radius", &(s->radius));
        change |= DisplayShaderInfo((SimpleShaderInfo *)s->shaderInfo);
    }

    if (activeObjectFlag == CUBE) {
        Cube *c = (Cube *)activeObjectPtr;
        change |= ImGui::DragFloat3("Center", (float *)&(c->center));
        change |= ImGui::DragFloat3("Size", (float *)&(c->size));
        change |= DisplayShaderInfo((SimpleShaderInfo *)c->shaderInfo);
    }

    if (activeObjectFlag == PLANE) {
        Plane *p = (Plane *)activeObjectPtr;
        change |= ImGui::DragFloat3("Center", (float *)&(p->center));
        change |= ImGui::DragFloat3("normal", (float *)&(p->normal));
        change |= DisplayShaderInfo((SimpleShaderInfo *)p->shaderInfo);
    }

    if (activeObjectFlag == TRIANGLE) {
        Triangle *t = (Triangle *)activeObjectPtr;
        change |= ImGui::DragFloat3("vertex 0", (float *)&(t->vertices[0]));
        change |= ImGui::DragFloat3("vertex 1", (float *)&(t->vertices[1]));
        change |= ImGui::DragFloat3("vertex 2", (float *)&(t->vertices[2]));
        change |= ImGui::DragFloat3("normal 0", (float *)&(t->normal[0]));
        change |= ImGui::DragFloat3("normal 1", (float *)&(t->normal[1]));
        change |= ImGui::DragFloat3("normal 2", (float *)&(t->normal[2]));
        change |= DisplayShaderInfo((SimpleShaderInfo *)t->shaderInfo);
    }

    if (activeObjectFlag == OBJECT) {
        Object *o = (Object *)activeObjectPtr;
        change |= DisplayShaderInfo((SimpleShaderInfo *)o->shaderInfo);
    }

    if (change)
        callReset();
    ImGui::End();
};

void displayObjects() {
    ImGui::Begin("Objects");
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1), "Objects");
    ImGui::BeginChild("Scrolling");
    float windowWidth = ImGui::GetWindowWidth();
    int numP = getNumPrimitives();
    void *primitive;
    for (int n = 0; n < numP; n++) {
        primitive = getPrimitive(n);
        char flag = *((char *)primitive);
        auto name = objectNames(flag) + std::to_string(n);
        if (ImGui::Button(name.c_str(), ImVec2(windowWidth, 0))) {
            activeObjectPtr = primitive;
            activeObject = name;
            activeObjectFlag = flag;
        }
    }
    ImGui::EndChild();
    ImGui::End();
    displayActiveObject();
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
    ImGui::End();
}

void createWindow() {
    SDL_Window *window;
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
    bool quit = false;
    bool preview = false;
    ImVec2 previewSize(0, 0);
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0,
                               255); // Clear the texture to black
        SDL_RenderClear(renderer);

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        // show rendering time
        auto tNow = std::chrono::high_resolution_clock::now();
        double elapsed_time_ms =
            std::chrono::duration<double, std::milli>(tNow - tBegin).count();
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
        ImGui::Begin("Menu");
        float windowWidth = ImGui::GetWindowWidth();
        if (ImGui::Button("Quit", ImVec2(windowWidth, 0))) {
            destroyTracer();
            break;
        }
        ImGui::TextColored(ImVec4(0.8, 0.8, 0.8, 1), "Window Settings");
        ImGui::Checkbox("Preview", &preview);
        ImGui::End();

        displayObjects();
        displayCamera();

        ImGui::Begin("Rendering", nullptr);
        if (preview) {
            ImGuiWindowFlags previewFlags = ImGuiWindowFlags_NoDecoration;
            ImVec2 renderingSize = ImGui::GetWindowSize();
            ImGui::End();
            ImGui::Begin("Preview", nullptr);
            ImVec2 previewSize = ImGui::GetWindowSize();
            float factor = (float)previewSize.x / renderingSize.x;
            ImGui::SetWindowSize(
                "Preview", ImVec2(previewSize.x, renderingSize.y * factor));
        }
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImVec2 size = ImGui::GetWindowSize();
        size = ImVec2(size.x, size.y);
        if ((int)size.x != WIDTH || (int)size.y != HEIGHT) {
            setWindowSize(size.x, size.y);
            WIDTH = getWindowSize().x;
            HEIGHT = getWindowSize().y;
            tBegin = std::chrono::high_resolution_clock::now();
        }
        // Draw pixels
        
        // ImDrawList* drawList = ImGui::GetWindowDrawList();
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                auto c = tracerGetPixel((int)(x),
                                        (int)(y));
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
