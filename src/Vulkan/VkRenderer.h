#ifndef VKRENDERER_H
#define VKRENDERER_H
#include "../common.h"
#include "../UI/ImguiModule.h"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float4.hpp"
#include <optional>
#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
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

struct RayState {
    glm::vec4 origin;
    glm::vec4 direction;
    glm::vec4 throughPut;
    glm::vec4 light;
    glm::vec4 pixelColor;
    u32 randomState;
    u32 terminated;
    u32 __padding[2];
};

struct alignas(16) Camera {
    glm::vec4 position;  // Camera position
    glm::vec4 forward; // Camera direction (where it's looking)
    glm::vec4 right; // Camera direction (where it's looking)
    glm::vec4 up;        // Up vector
    float fov;          // Field of view
    float yaw;
    float pitch;
    u32 reset;
    //shading helper - volumetric and MIS
    u32 lightCount;
    u32 nee;
    u32 padding[2];
    void rotate(glm::vec2 mouse, float deltaTime);
    void move(glm::vec2 control, float deltaTime);
    void setNewFOV(float deg);
    Camera();
};

struct Material {
    glm::vec4 color;
    glm::vec4 textureData; //offset, width, height, 
    float emission;
    float ior1 = 1.0f;
    float ior2 = 1.5f;
    u32 shaderFlag;
};

struct Vertex {
    glm::vec4 position;
    glm::vec4 normal;
    glm::vec2 uv;
    u32 materialIdx;
    u32 padding;
    bool operator==(const Vertex& other) const;
};
template<> struct std::hash<Vertex> {
    size_t operator()(Vertex const& vertex) const;
};
class VkRenderer {
public:
    void run();
//private:
    //functions
    void initWindow();
    void initVulkan();
    void createInstance();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSurface();
    void createSwapChain();
    void createImageViews();
    void createRaytracingPipeline();
    void createDescriptorLayout();
    void createCommandPool();
    void createFramebuffers();
    void createRenderPasses();
    void createDescriptorPool();
    void createCommandBuffer();
    void beginCommandBuffer();
    void endCommandBuffer();
    void loadGeometry();
    void updateDescriptorSet();
    void buildAccelerationStructures();
    void handleInput(float deltaTime);
    void buildBottomLevelAS();
    void buildTopLevelAS();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, float deltaTime);
    void copyDataToBuffer(VkDeviceMemory bufferMemory, const void* shaderCode, VkDeviceSize shaderSize);
    void copyTracedImageToSwapchain(int imageIndex);
    void createGeometryBuffers();
    void traceImage();
    void mainLoop();
    void createSyncObjects();
    void drawFrame(float deltaTime);
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
    VkDeviceAddress getBufferAdress(const VkBuffer & bufferHandle);
    
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
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSets[3];
    VkDescriptorSetLayout descriptorSetLayouts[3];
    VkRenderPass renderPass;
    ImguiModule gui;
    std::vector<RayState> waveFront;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    //buffers
    VkBuffer raygenBuffer, missBuffer, hitBuffer;
    VkBuffer vertexBuffer;
    VkBuffer textureBuffer;
    VkBuffer materialBuffer;
    VkBuffer indexBuffer;
    VkBuffer emissiveBuffer;
    VkBuffer waveFrontBuffer;
    VkBuffer bottomLevelASBuffer, topLevelASBuffer, instanceASBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkDeviceMemory indexBufferMemory;
    VkDeviceMemory waveFrontBufferMemory;
    VkDeviceMemory materialBufferMemory;
    VkDeviceMemory textureBufferMemory;
    VkDeviceMemory emissiveBufferMemory;
    VkDeviceMemory raygenMemory, missMemory, hitMemory;
    VkDeviceMemory bottomLevelASMemory, topLevelASMemory, instanceASMemory;
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    std::vector<Material> materials;
    std::vector<std::string> materialNames;
    std::vector<u32> emissiveTriangles;

    //one giant textureAtlas with offsets and so on in the material
    std::vector<glm::vec4> textureAtlas;

    //Shaders
    VkShaderModule rgenShaderModule, closesthitShaderModule, missShaderModule;
    VkDeviceSize shaderGroupBaseAlignment;
    VkDeviceSize shaderGroupHandleSize;
    VkStridedDeviceAddressRegionKHR raygenSBT{};
    VkStridedDeviceAddressRegionKHR missSBT{};
    VkStridedDeviceAddressRegionKHR hitSBT{};
    VkStridedDeviceAddressRegionKHR callableSBT{};


    //image to render too
    VkImage storageImage;
    VkImageView storageImageView;
    VkDeviceMemory imageMemory;
    
    //Camera
    void updateCamera(glm::vec2 mouse, glm::vec2 control, float deltaTime);
    void createCamera();
    VkBuffer cameraBuffer;
    VkDeviceMemory cameraBufferMemory;
    Camera camera;


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
    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR = nullptr;
    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR = nullptr;
};

//Helper function
inline void checkIfVkResultIsCorrect(const VkResult & result, const char * error) {
    if(result == VK_SUCCESS) return;
    throw std::runtime_error(error);
}


#endif // !VKRENDERER_H
