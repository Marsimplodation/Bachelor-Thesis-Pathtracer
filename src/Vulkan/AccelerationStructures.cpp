#include "VkRenderer.h"
#include <vector>
#include <vulkan/vulkan_core.h>
#include <glm/vec3.hpp>

void VkRenderer::buildAccelerationStructures() {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    // Triangle vertex data (positions and colors)
    std::vector<Vertex> vertices = {
        {{0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Vertex 0
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Vertex 1
        {{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}   // Vertex 2
    };
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(vertices[0]) * vertices.size();
    bufferInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    VkAccelerationStructureCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    createInfo.size = sizeof(vertices[0]) * vertices.size();
    createInfo.buffer = vertexBuffer;

    VkAccelerationStructureKHR topLevelAS;
    VkResult result = vkCreateAccelerationStructureKHR(device, &createInfo, nullptr, &topLevelAS);
    checkIfVkResultIsCorrect(result, "Failed to create empty acceleration structure");
}

