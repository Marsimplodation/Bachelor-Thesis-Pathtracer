#include "VkRenderer.h"
#include <vulkan/vulkan_core.h>

void VkRenderer::createLogicalDevice() {
    auto indices = QueueFamilyIndices::findQueueFamilies(this->physicalDevice);
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    float queuePrio = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePrio;

    VkPhysicalDeviceFeatures deviceFeatures{};
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;
    
    //compatiblity reasons, no longer needed
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
    auto result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
    checkIfVkResultIsCorrect(result, "failed to create logical deivce");
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
}
