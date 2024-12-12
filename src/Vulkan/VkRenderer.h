#ifndef VKRENDERER_H
#include "../common.h"
#include <optional>
#include "GLFW/glfw3.h"
#include "../common.h"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

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
    void createLogicalDevice();
    void mainLoop();
    void cleanup();

    //members
    GLFWwindow* window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    
    //validation
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    #ifdef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif
};

//Helper function
inline void checkIfVkResultIsCorrect(const VkResult & result, const char * error) {
    if(result == VK_SUCCESS) return;
    throw std::runtime_error(error);
}


#endif // !VKRENDERER_H
