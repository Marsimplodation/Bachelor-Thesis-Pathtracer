#include "window.h"
#include <SDL2/SDL.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include <imgui.h>
#include <iostream>
#include <vector>
#include "../tracer.h"

namespace {
    std::vector<visualInformation> fields = std::vector<visualInformation>();
}


void registerInfo(float & f, const char * name){
    fields.push_back({
        .data = &f,
        .name = name,
        .flag = 0x00,
    });
}

void registerInfo(Vector3 & v, const char * name){
    fields.push_back({
        .data = &v,
        .name = name,
        .flag = 0x01,
    });
}

void createWindow() {
    SDL_Window *window;
    SDL_Init(SDL_INIT_VIDEO);
    int WIDTH = getWindowSize().x;
    int HEIGHT = getWindowSize().y;
    window =
        SDL_CreateWindow("Pixel Drawing", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, WIDTH + 200, HEIGHT, SDL_INIT_VIDEO);
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
    bool preview = true;
    int scaleFactor = 1;
    int zoomFactor = 1;
    int zoomOffset[] = {0,0};
    ImVec2 previewSize(0,0);
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
        

        ImGui::Begin("Menu");
        if(ImGui::Button("clear")) {
           reset(); 
        }
        ImGui::Checkbox("Preview", &preview);
        ImGui::DragInt("scale factor", &scaleFactor);
        ImGui::DragInt("zoom factor", &zoomFactor);
        ImGui::DragInt2("zoom offset", zoomOffset);

        for(auto & var : fields) {
           switch (var.flag) {
                case 0x00: 
                    if(ImGui::DragFloat(var.name, (float*)var.data)) reset();
                    break;
                case 0x01:
                    if(ImGui::DragFloat3(var.name, (float*)var.data)) reset();
                default:
                    break;
           }
        }
        ImGui::End();


        ImGui::Begin("Rendering", nullptr);
        if(preview) {
            ImVec2 renderingSize = ImGui::GetWindowSize();
            ImGui::End();
            ImGui::Begin("Preview", nullptr);
            ImVec2 previewSize = ImGui::GetWindowSize();
            float factor = (float)previewSize.x / renderingSize.x;
            ImGui::SetWindowSize("Preview", ImVec2(previewSize.x, renderingSize.y * factor));
        }
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImVec2 size = ImGui::GetWindowSize();
        size = ImVec2(size.x/scaleFactor, size.y/scaleFactor);
        if((int)size.x != WIDTH || (int)size.y != HEIGHT) {
            setWindowSize(size.x, size.y);
            WIDTH = getWindowSize().x;
            HEIGHT = getWindowSize().y;
        }
        // Draw pixels
        int steps = scaleFactor*zoomFactor;
        for (int x = 0; x < WIDTH*steps; x+=steps)
        {
            for (int y = 0; y < HEIGHT*steps; y+=steps)
            {
                ImVec2 pixelPos = ImVec2(canvasPos.x + x, canvasPos.y + y);
                auto c = tracerGetPixel((int)(x+zoomOffset[0])/steps, (int)(y+zoomOffset[1])/steps);
                c = clampToOne(c)*255;
                for (int i = 1; i <=steps; i++) {
                    for (int j = 1; j <=steps; j++) {
                        ImGui::GetWindowDrawList()->AddRectFilled(pixelPos, ImVec2(pixelPos.x + i, pixelPos.y + j), IM_COL32(c.x, c.y, c.z, 255));
                    }
                } 
            }
        }
        ImGui::End();


        
        

        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

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
