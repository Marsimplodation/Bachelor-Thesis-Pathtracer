#include "VkRenderer.h"

bool VkRenderer::isDeviceSuitable(VkPhysicalDevice device){
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceProperties(device, &properties);
    vkGetPhysicalDeviceFeatures(device, &features);
    QueueFamilyIndices indices = findQueueFamilies(device);
    return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            features.geometryShader &&
            indices.isComplete();
};

void VkRenderer::pickPhysicalDevice() {
    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr); 
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for(const auto & device : devices) {
        if(!isDeviceSuitable(device))continue;
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        physicalDevice = device;
        printf("Picked Vulkan device: %s\n", properties.deviceName);
        break;
    }

}
