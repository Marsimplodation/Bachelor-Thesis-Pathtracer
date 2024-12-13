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
    VkBuffer vertexBuffer;
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
    createInfo.size = 0;  // No geometry, just an empty AS
    createInfo.buffer = vertexBuffer;

    VkAccelerationStructureKHR topLevelAS;
    VkResult result = vkCreateAccelerationStructureKHR(device, &createInfo, nullptr, &topLevelAS);
    checkIfVkResultIsCorrect(result, "Failed to create empty acceleration structure");

    // Create descriptor set layout for the acceleration structure (if you plan to use it later)
    VkDescriptorSetLayoutBinding asBinding = {};
    asBinding.binding = 0;
    asBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    asBinding.descriptorCount = 1;
    asBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = {};
    setLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setLayoutCreateInfo.bindingCount = 1;
    setLayoutCreateInfo.pBindings = &asBinding;

    result = vkCreateDescriptorSetLayout(device, &setLayoutCreateInfo, nullptr, &descriptorSetLayout);
    checkIfVkResultIsCorrect(result, "Failed to create descriptor set layout for AS");

    // Since this is just an empty AS for now, we don't need any further steps
    // to bind the AS to the descriptor set or allocate memory for it
}

