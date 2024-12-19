#include "ImguiModule.h"
#include "GLFW/glfw3.h"
#include <cmath>
#include <iterator>
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


void ImguiModule::update(VkCommandBuffer commandBuffer, float deltaTime) {
        if(!active) return;
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame(); // If using GLFW
        ImGui::NewFrame();

        // Build your GUI
        ShowFPSOverlay(deltaTime);
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
}
