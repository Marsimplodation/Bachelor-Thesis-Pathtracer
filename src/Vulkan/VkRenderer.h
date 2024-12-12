#ifndef VKRENDERER_H
#include "../common.h"
#include <optional>

#include <cstdio>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

struct QueueFamilyIndices {
    std::optional<u32> graphicsFamily;
    bool isComplete();
    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
};

class VkRenderer {
public:
    void run();
private:
    //functions
    void initWindow();
    void initVulkan();
    bool checkValidationLayerSupport();
    void createInstance();
    void pickPhysicalDevice();
    void mainLoop();
    void cleanup();

    //members
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    GLFWwindow* window;
};


#endif // !VKRENDERER_H
