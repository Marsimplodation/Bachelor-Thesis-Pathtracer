#include "ImguiModule.h"
#include "GLFW/glfw3.h"
#include <cmath>
#include <iterator>
#include "../Vulkan/VkRenderer.h"
#include "glm/detail/qualifier.hpp"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>


void customTheme() {
auto &colors = ImGui::GetStyle().Colors;
colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};
colors[ImGuiCol_MenuBarBg] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

// Border
colors[ImGuiCol_Border] = ImVec4{0.44f, 0.37f, 0.61f, 0.29f};
colors[ImGuiCol_BorderShadow] = ImVec4{0.0f, 0.0f, 0.0f, 0.24f};

// Text
colors[ImGuiCol_Text] = ImVec4{1.0f, 1.0f, 1.0f, 1.0f};
colors[ImGuiCol_TextDisabled] = ImVec4{0.5f, 0.5f, 0.5f, 1.0f};

// Headers
colors[ImGuiCol_Header] = ImVec4{0.13f, 0.13f, 0.17, 1.0f};
colors[ImGuiCol_HeaderHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
colors[ImGuiCol_HeaderActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

// Buttons
colors[ImGuiCol_Button] = ImVec4{0.13f, 0.13f, 0.17, 1.0f};
colors[ImGuiCol_ButtonHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
colors[ImGuiCol_ButtonActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
colors[ImGuiCol_CheckMark] = ImVec4{0.74f, 0.58f, 0.98f, 1.0f};

// Popups
colors[ImGuiCol_PopupBg] = ImVec4{0.1f, 0.1f, 0.13f, 0.92f};

// Slider
colors[ImGuiCol_SliderGrab] = ImVec4{0.44f, 0.37f, 0.61f, 0.54f};
colors[ImGuiCol_SliderGrabActive] = ImVec4{0.74f, 0.58f, 0.98f, 0.54f};

// Frame BG
colors[ImGuiCol_FrameBg] = ImVec4{0.13f, 0.13, 0.17, 1.0f};
colors[ImGuiCol_FrameBgHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
colors[ImGuiCol_FrameBgActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

// Tabs
colors[ImGuiCol_Tab] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
colors[ImGuiCol_TabHovered] = ImVec4{0.24, 0.24f, 0.32f, 1.0f};
colors[ImGuiCol_TabActive] = ImVec4{0.2f, 0.22f, 0.27f, 1.0f};
colors[ImGuiCol_TabUnfocused] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

// Title
colors[ImGuiCol_TitleBg] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
colors[ImGuiCol_TitleBgActive] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};

// Scrollbar
colors[ImGuiCol_ScrollbarBg] = ImVec4{0.1f, 0.1f, 0.13f, 1.0f};
colors[ImGuiCol_ScrollbarGrab] = ImVec4{0.16f, 0.16f, 0.21f, 1.0f};
colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{0.19f, 0.2f, 0.25f, 1.0f};
colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{0.24f, 0.24f, 0.32f, 1.0f};

// Seperator
colors[ImGuiCol_Separator] = ImVec4{0.44f, 0.37f, 0.61f, 1.0f};
colors[ImGuiCol_SeparatorHovered] = ImVec4{0.74f, 0.58f, 0.98f, 1.0f};
colors[ImGuiCol_SeparatorActive] = ImVec4{0.84f, 0.58f, 1.0f, 1.0f};

// Resize Grip
colors[ImGuiCol_ResizeGrip] = ImVec4{0.44f, 0.37f, 0.61f, 0.29f};
colors[ImGuiCol_ResizeGripHovered] = ImVec4{0.74f, 0.58f, 0.98f, 0.29f};
colors[ImGuiCol_ResizeGripActive] = ImVec4{0.84f, 0.58f, 1.0f, 0.29f};

// Docking
colors[ImGuiCol_DockingPreview] = ImVec4{0.44f, 0.37f, 0.61f, 1.0f};

auto &style = ImGui::GetStyle();
style.TabRounding = 4;
style.ScrollbarRounding = 9;
style.WindowRounding = 7;
style.GrabRounding = 3;
style.FrameRounding = 3;
style.PopupRounding = 4;
style.ChildRounding = 4;
  // Go through every colour and convert it to linear
  // This is because ImGui uses linear colours but we are using sRGB
  // This is a simple approximation of the conversion
  for (int i = 0; i < ImGuiCol_COUNT; i++) {
    /*float linear = (srgb <= 0.04045f) ? srgb / 12.92f : pow((srgb + 0.055f)
     * / 1.055f, 2.4f);*/

    ImVec4 &col = style.Colors[i];
    col.x = col.x <= 0.04045f ? col.x / 12.92f
                              : pow((col.x + 0.055f) / 1.055f, 2.4f);
    col.y = col.y <= 0.04045f ? col.y / 12.92f
                              : pow((col.y + 0.055f) / 1.055f, 2.4f);
    col.z = col.z <= 0.04045f ? col.z / 12.92f
                              : pow((col.z + 0.055f) / 1.055f, 2.4f);
  }
}

void ImguiModule::init(VkDevice device,
                        VkPhysicalDevice physicalDevice,
                        VkInstance instance,
                        VkQueue graphicsQueue,
                        VkRenderPass renderPass,
                        int imageCount,
                        GLFWwindow * window,
                        void * rendererPtr
                       ) {
    VkDescriptorPoolSize pool_sizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VkResult result = vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiPool);
    
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = instance;
    init_info.PhysicalDevice = physicalDevice;
    init_info.Device = device;
    init_info.Queue = graphicsQueue;
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = imageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.RenderPass = renderPass;
    ImGui_ImplVulkan_Init(&init_info);
    const VkFormat requested = {VK_FORMAT_B8G8R8A8_SRGB};
    VkRenderer & renderer = *(VkRenderer*)rendererPtr;
    ImGui_ImplVulkanH_SelectSurfaceFormat(physicalDevice, renderer.surface, &requested, 1, VK_COLORSPACE_SRGB_NONLINEAR_KHR);
    ImGui_ImplGlfw_InitForVulkan(window, true);
    customTheme();

}

void ImguiModule::destroy(VkDevice device) {
    // Cleanup (happens once)
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(device, imguiPool, nullptr);
}


void ShowFPSOverlay(float deltaTime) {
    // Calculate FPS
    float f_fps = 1.0f / deltaTime;
    int fps = static_cast<int>(std::floor(f_fps));

    // Create overlay
    ImGui::SetNextWindowPos(ImVec2(10, 10)); // Top-left corner
    ImGui::SetNextWindowBgAlpha(0.0f);       // Transparent background

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |    // No title bar, resize, etc.
                             ImGuiWindowFlags_AlwaysAutoResize | // Automatically fit to text size
                             ImGuiWindowFlags_NoSavedSettings |  // Don't save settings to .ini file
                             ImGuiWindowFlags_NoFocusOnAppearing |
                             ImGuiWindowFlags_NoNav;             // Disable navigation controls

    if (ImGui::Begin("FPS Overlay", nullptr, flags)) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // Orange text
        ImGui::Text("FPS: %d", fps);
        ImGui::PopStyleColor(); // Revert color change
    }
    ImGui::End();
}

float defaultCameraFov = 70;
bool showCamera(VkRenderer& renderer) {
    bool changed = false;
    ImGui::Begin("Camera");

    if(ImGui::DragFloat("FOV", &defaultCameraFov)) {
        changed = true;
        renderer.camera.setNewFOV(defaultCameraFov);;
    }
    changed |= ImGui::Checkbox("Volumetric Fog", (bool*)&renderer.camera.nee);
    changed |= ImGui::DragFloat("Fog Density", &renderer.camera.fogDensity);
    changed |= ImGui::DragFloat("Fog Scale", &renderer.camera.fogScale);
    ImGui::End();
    return changed;
}

int selectedMaterial = 0;
bool showMaterials(VkRenderer& renderer) {
    bool changed = false;
    enum ShaderFlags {
        Diffuse = 0x00,
        Mirror = 0x01,
        Refraction = 0x02,
    };
    // For displaying flag names in the dropdown
    std::vector<std::string> shaderFlagNames = {
    "Diffuse",       // 0x00
    "Mirror",        // 0x01
    "Refraction",        // 0x01
    };
    ImGui::Begin("Materials");
    
    Material & material = renderer.materials[selectedMaterial];
    #define GAMMA 2.2
    #define INV_GAMMA 0.4545f 

    glm::vec4 sdrColor = material.color;
    sdrColor[0] = std::powf(sdrColor[0], INV_GAMMA);
    sdrColor[1] = std::powf(sdrColor[1], INV_GAMMA);
    sdrColor[2] = std::powf(sdrColor[2], INV_GAMMA);
    if(ImGui::ColorEdit4("Color", (float*)&sdrColor)) {
        material.color[0] = std::powf(sdrColor[0], GAMMA);
        material.color[1] = std::powf(sdrColor[1], GAMMA);
        material.color[2] = std::powf(sdrColor[2], GAMMA);
        changed = true;
    }

    float oldEmission = material.emission;
    changed |= ImGui::DragFloat("Emission", &material.emission);
    changed |= ImGui::DragFloat("ior 1", &material.ior1);
    changed |= ImGui::DragFloat("ior 2", &material.ior2);
    changed |= ImGui::DragFloat4("tex data", (float*)&material.textureData);
   // Use the flag's current value to index the name
    ShaderFlags currentFlag = (ShaderFlags)material.shaderFlag;
    std::string currentFlagName = shaderFlagNames[currentFlag];

    if (ImGui::BeginCombo("Shader Flags", currentFlagName.c_str())) {
        for (size_t i = 0; i < shaderFlagNames.size(); ++i) {
            bool isSelected = (currentFlag == static_cast<int>(i));
            if (ImGui::Selectable(shaderFlagNames[i].c_str(), isSelected)) {
                material.shaderFlag = (i);
                changed = true;
            }

            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }   


    int i = 0;
    for (auto & name : renderer.materialNames) {
        if(ImGui::Button(name.c_str())) {selectedMaterial = i;}
        i++;
    }
    ImGui::End();

    //push new emissive triangles
    if(oldEmission == 0.0f && oldEmission != material.emission) {
        for (int i = 0; i < renderer.indices.size(); i+=3) {
            Vertex & v0 = renderer.vertices[renderer.indices[i]];
            if(v0.materialIdx != selectedMaterial) continue;
            renderer.emissiveTriangles.push_back(i);
        }
        if(renderer.emissiveTriangles.size() > 0)
        renderer.copyDataToBuffer(renderer.emissiveBufferMemory, renderer.emissiveTriangles.data(), sizeof(u32)*renderer.emissiveTriangles.size());
    }
    //remove emissive triangles
    if(material.emission == 0.0f && oldEmission != material.emission) {
        renderer.emissiveTriangles.clear();
        for (int i = 0; i < renderer.indices.size(); i+=3) {
            Vertex & v0 = renderer.vertices[renderer.indices[i]];
            if(renderer.materials[v0.materialIdx].emission == 0.0f) continue;
            renderer.emissiveTriangles.push_back(i);
        }
        
        if(renderer.emissiveTriangles.size() > 0)
        renderer.copyDataToBuffer(renderer.emissiveBufferMemory, renderer.emissiveTriangles.data(), sizeof(u32)*renderer.emissiveTriangles.size());

    }

    if(changed){
        //repush materials + rerender
        renderer.copyDataToBuffer(renderer.materialBufferMemory, renderer.materials.data(), sizeof(Material)*renderer.materials.size());
    }
    return changed;
}
bool textureCreated = false;
void ImguiModule::update(void* rendererPtr, float deltaTime) {
        VkRenderer & renderer = *(VkRenderer*)rendererPtr;

        if(textureCreated) {
            ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(textureID));
        }
            textureID = reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(
                renderer.imageSampler,
                renderer.storageImageView,                       // VkSampler
                VK_IMAGE_LAYOUT_GENERAL// Image layout for sampling
            ));
            textureCreated = true;

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame(); // If using GLFW
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0);

        // Build your GUI
        ShowFPSOverlay(deltaTime);
        ImGui::Begin("Viewport");
        auto size = ImGui::GetWindowSize();
        float ratio = renderer.swapChainExtent.width / renderer.swapChainExtent.height;
        size.y = size.x * ratio;
        ImGui::SetWindowSize(size);
        
        ImGui::Image(textureID,size);

        ImGui::End();
        updated |= showMaterials(renderer);
        updated |= showCamera(renderer);
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, renderer.commandBuffer);
}
