#ifndef VKRENDERER_H
#include "../common.h"
#include <optional>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../common.h"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

struct QueueFamilyIndices {
    std::optional<u32> graphicsFamily;
    std::optional<u32> presentFamily;
    bool isComplete();
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VkRenderer {
public:
    void run();
private:
    //functions
    void initWindow();
    void initVulkan();
    void createInstance();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSurface();
    void createSwapChain();
    void createImageViews();
    void createGraphicsPipeline();
    void createDescriptorLayout();
    void createCommandPool();
    void createFramebuffers();
    void createRenderPasses();
    void createDescriptorPool();
    void createCommandBuffer();
    void updateDescriptorSet();
    void buildAccelerationStructures();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void mainLoop();
    void createSyncObjects();
    void drawFrame();
    void cleanup();
    bool checkValidationLayerSupport();
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    
    u32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties);
    VkResult  CreateBuffer(VkDeviceSize size,
                 VkBufferUsageFlags usage,
                 VkMemoryPropertyFlags properties,
                VkBuffer* buffer,
                 VkDeviceMemory* bufferMemory,
                 bool requiresDeviceAddress = false);
    VkResult CreateSBTBuffers(VkDeviceSize raygenSize, VkDeviceSize missSize, VkDeviceSize hitSize,
                              VkBuffer* raygenBuffer, VkDeviceMemory* raygenMemory,
                              VkBuffer* missBuffer, VkDeviceMemory* missMemory,
                              VkBuffer* hitBuffer, VkDeviceMemory* hitMemory);



    //members
    GLFWwindow* window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkPipelineLayout pipelineLayout;
    VkPipeline rtPipeline;
    VkDescriptorSetLayout descriptorSetLayout;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkRenderPass renderPass;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    //buffers
    VkBuffer raygenBuffer, missBuffer, hitBuffer;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkDeviceMemory raygenMemory, missMemory, hitMemory;

    //Shaders
    VkShaderModule rgenShaderModule, closesthitShaderModule, missShaderModule;
    VkDeviceSize shaderGroupBaseAlignment;
    VkDeviceSize shaderGroupHandleSize;

    //image to render too
    VkImage storageImage;
    VkImageView storageImageView;
    VkDeviceMemory imageMemory;


    VkAccelerationStructureKHR topLevelAS;
    VkAccelerationStructureKHR bottomLevelAS;

    
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_KHR_SPIRV_1_4_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
    };
    
    //validation
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    #ifdef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif

    //extensions
    PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR = nullptr;
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = nullptr;
    PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = nullptr;
    PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR = nullptr;
    PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR  = nullptr;
    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR = nullptr;
    PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR = nullptr;
};

//Helper function
inline void checkIfVkResultIsCorrect(const VkResult & result, const char * error) {
    if(result == VK_SUCCESS) return;
    throw std::runtime_error(error);
}


#endif // !VKRENDERER_H
