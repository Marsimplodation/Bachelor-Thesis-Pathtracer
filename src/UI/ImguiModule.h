#ifndef IMGUI_MODULE_H
#define IMGUI_MODULE_H 
#include "imgui.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan_core.h>
class ImguiModule {
    public:
    void init(VkDevice device,
                        VkPhysicalDevice physicalDevice,
                        VkInstance instance,
                        VkQueue graphicsQueue,
                        VkRenderPass renderPass,
                        int imageCount,
                        GLFWwindow * window,
                        void * renderer
                       );
        void update(void* render, float deltaTime);
        void destroy(VkDevice device);
        bool active = false;
        bool updated = false;
        ImTextureID textureID;
    private:
        VkDescriptorPool imguiPool;
};
#endif // !IMGUI_MODULE_H
