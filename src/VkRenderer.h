#ifndef VKRENDERER_H

#include <cstdio>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
class VkRenderer {
public:
    void run();
private:
    //functions
    void initWindow();
    void initVulkan();
    bool checkValidationLayerSupport();
    void createInstance();
    void mainLoop();
    void cleanup();

    //members
    VkInstance instance;
    GLFWwindow* window;
};
#endif // !VKRENDERER_H
