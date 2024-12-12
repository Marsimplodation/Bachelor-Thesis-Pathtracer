#include "VkRenderer.h"
void VkRenderer::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

//--- PRIVATE ---//

void VkRenderer::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
}

void VkRenderer::initVulkan() {
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
}

void VkRenderer::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        // Poll for events
        glfwPollEvents();

        // Check if 'q' key is pressed
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);  // Close the window
        }

        // Add your rendering code here (e.g., clear the window, draw stuff, etc.)

        // Swap buffers to update the window
        glfwSwapBuffers(window);
    }

}

void VkRenderer::cleanup() {
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}


