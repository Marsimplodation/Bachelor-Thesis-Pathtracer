#include "VkRenderer.h"
#include "GLFW/glfw3.h"
#include "../common.h"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

bool isDeviceSuitable(VkPhysicalDevice device){
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceProperties(device, &properties);
    vkGetPhysicalDeviceFeatures(device, &features);
    QueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(device);
    return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            features.geometryShader &&
            indices.isComplete();
};

void VkRenderer::pickPhysicalDevice() {
    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr); 
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(this->instance, &deviceCount, devices.data());

    for(const auto & device : devices) {
        if(!isDeviceSuitable(device))continue;
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        this->physicalDevice = device;
        printf("Picked Vulkan device: %s\n", properties.deviceName);
        break;
    }

}
