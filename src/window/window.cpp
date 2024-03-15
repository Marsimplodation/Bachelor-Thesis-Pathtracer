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
        // Draw a pixel (e.g., red) at coordinates (400, 300)
        /*for(int x = 0; x < WIDTH; x++) {
            for(int y = 0; y < HEIGHT; y++) {
                auto c = tracerGetPixel(x, y);
                SDL_SetRenderDrawColor(renderer, c->x*255, c->y * 255, c->z*255, 255); 
                SDL_RenderDrawPoint(renderer, x, y + 100);
            }
        }*/
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        ImGui::Begin("Canvas", nullptr);

        ImVec2 canvasSize = ImGui::GetContentRegionAvail();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        
        ImVec2 size = ImGui::GetWindowSize();
        if(size.x != WIDTH || size.y != HEIGHT) {
            setWindowSize(size.x, size.y);
            WIDTH = getWindowSize().x;
            HEIGHT = getWindowSize().y;
        }
        // Draw pixels
        for (int x = 0; x < WIDTH; x++)
        {
            for (int y = 0; y < HEIGHT; y++)
            {
                ImVec2 pixelPos = ImVec2(canvasPos.x + x, canvasPos.y + y);
                auto c = tracerGetPixel(x, y) * 255;
                ImGui::GetWindowDrawList()->AddRectFilled(pixelPos, ImVec2(pixelPos.x + 1, pixelPos.y + 1), IM_COL32(c.x, c.y, c.z, 255));
            }
        }
        ImGui::End();


        
        ImGui::Begin("Menu");
        if(ImGui::Button("clear")) {
           reset(); 
        }

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
