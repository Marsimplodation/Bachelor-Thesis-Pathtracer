#include "ImguiModule.h"
#include "GLFW/glfw3.h"
#include <cmath>
#include <iterator>
#include "../Vulkan/VkRenderer.h"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>

void ImguiModule::init(VkDevice device,
                        VkPhysicalDevice physicalDevice,
                        VkInstance instance,
                        VkQueue graphicsQueue,
                        VkRenderPass renderPass,
                        int imageCount,
                        GLFWwindow * window
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
    ImGui_ImplGlfw_InitForVulkan(window, true);

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

bool showCamera(VkRenderer& renderer) {
    bool changed = false;
    ImGui::Begin("Camera");

    changed |= ImGui::DragFloat("FOV", (float*)&renderer.camera.fov);
    ImGui::End();
    return changed;
}

int selectedMaterial = 0;
bool showMaterials(VkRenderer& renderer) {
    bool changed = false;
    enum ShaderFlags {
        Diffuse = 0x00,
        Mirror = 0x01,
    };
    // For displaying flag names in the dropdown
    std::vector<std::string> shaderFlagNames = {
    "Diffuse",       // 0x00
    "Mirror",        // 0x01
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

    changed |= ImGui::DragFloat("Emission", &material.emission);
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

    if(changed){
        //repush materials + rerender
        renderer.copyDataToBuffer(renderer.materialBufferMemory, renderer.materials.data(), sizeof(Material)*renderer.materials.size());
    }
    return changed;
}

void ImguiModule::update(void* rendererPtr, float deltaTime) {
        VkRenderer & renderer = *(VkRenderer*)rendererPtr;
        if(!active) return;
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame(); // If using GLFW
        ImGui::NewFrame();

        // Build your GUI
        ShowFPSOverlay(deltaTime);
        updated |= showMaterials(renderer);
        updated |= showCamera(renderer);
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, renderer.commandBuffer);
}
