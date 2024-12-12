#include "VkRenderer.h"

void VkRenderer::createLogicalDevice() {
    auto indices = QueueFamilyIndices::findQueueFamilies(this->physicalDevice);
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    float queuePrio = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePrio;
}
