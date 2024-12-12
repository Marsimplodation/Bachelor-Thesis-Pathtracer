#include "VkRenderer.h"

bool QueueFamilyIndices::isComplete() {
    return this->graphicsFamily.has_value();
}

QueueFamilyIndices QueueFamilyIndices::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        if(indices.isComplete()) break;
        i++;
    }
    return indices;
};
