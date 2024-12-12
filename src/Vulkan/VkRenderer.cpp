#include "VkRenderer.h"
#include "GLFW/glfw3.h"
#include "../common.h"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>


namespace {
    inline void checkIfResultIsCorrect(const VkResult & result, const char * error) {
        if(result == VK_SUCCESS) return;
        throw std::runtime_error(error);
    }
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    

    #ifdef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif
}

void VkRenderer::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}
void VkRenderer::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    this->window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
}

bool VkRenderer::checkValidationLayerSupport() {
    u32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    for (const char* layerName: validationLayers) {
        bool found = false;
        for(const auto & properties : availableLayers){
            if(strcmp(layerName, properties.layerName) != 0) continue;
            found = true;
            break;
        }
        if(!found) return false;
    }
    return true;
}

void VkRenderer::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    } else if(enableValidationLayers){printf("Validation active\n");}

    VkApplicationInfo appinfo{};
    appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appinfo.pApplicationName = "Vulkan Renderer";
    appinfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appinfo.pEngineName = "No Engine";
    appinfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appinfo.apiVersion = VK_API_VERSION_1_4;
    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appinfo;

    u32 glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    
    if(enableValidationLayers) {
        createInfo.enabledLayerCount = validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else createInfo.enabledLayerCount = 0;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &this->instance);
    checkIfResultIsCorrect(result, "Failed to create instance"); 
}



void VkRenderer::initVulkan() {
    createInstance();
    pickPhysicalDevice();
}

void VkRenderer::mainLoop() {
    while (!glfwWindowShouldClose(this->window)) {
        // Poll for events
        glfwPollEvents();

        // Check if 'q' key is pressed
        if (glfwGetKey(this->window, GLFW_KEY_Q) == GLFW_PRESS) {
            glfwSetWindowShouldClose(this->window, GLFW_TRUE);  // Close the window
        }

        // Add your rendering code here (e.g., clear the window, draw stuff, etc.)

        // Swap buffers to update the window
        glfwSwapBuffers(this->window);
    }

}

void VkRenderer::cleanup() {
    vkDestroyInstance(this->instance, nullptr);
    glfwDestroyWindow(this->window);
    glfwTerminate();
}


