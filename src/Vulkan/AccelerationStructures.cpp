#include "VkRenderer.h"
#include <vector>
#include <vulkan/vulkan_core.h>
#include <glm/vec3.hpp>

void VkRenderer::buildAccelerationStructures() {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    //create AS BUFFERS
    CreateBuffer(sizeof(VkAccelerationStructureKHR),
                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                 &bottomLevelASBuffer,
                 &bottomLevelASMemory, true);
    CreateBuffer(sizeof(VkAccelerationStructureKHR),
                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                 &topLevelASBuffer,
                 &topLevelASMemory, true);



    // Triangle vertex data (positions and colors)
    CreateBuffer(sizeof(Vertex) * 3,
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                 &vertexBuffer,
                 &vertexBufferMemory, true);


    std::vector<Vertex> vertices = {
        {{0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Vertex 0
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Vertex 1
        {{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}   // Vertex 2
    };
    copyDataToBuffer(vertexBufferMemory, vertices.data(), sizeof(Vertex)*3);
    VkBufferDeviceAddressInfo bufferDeviceAddressInfo{};
    bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
    bufferDeviceAddressInfo.buffer = vertexBuffer;
    VkDeviceAddress vertexBufferDeviceAddress = vkGetBufferDeviceAddressKHR(device, &bufferDeviceAddressInfo);



    //--- BOTTOM LEVEL ---//
    VkAccelerationStructureCreateInfoKHR blasCreateInfo = {};
    blasCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    blasCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    blasCreateInfo.size = sizeof(VkAccelerationStructureKHR);
    blasCreateInfo.buffer = bottomLevelASBuffer;


    VkResult result = vkCreateAccelerationStructureKHR(device, &blasCreateInfo, nullptr, &bottomLevelAS);
    checkIfVkResultIsCorrect(result, "Failed to create empty acceleration structure");


    //---- TOP LEVEL ----//
    VkAccelerationStructureCreateInfoKHR tlasCreateInfo = {};
    tlasCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    tlasCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    tlasCreateInfo.size = sizeof(VkAccelerationStructureKHR);
    tlasCreateInfo.buffer = topLevelASBuffer;

    /*
    vkBuildAccelerationStructuresKHR(
        VkDevice device,
        VkDeferredOperationKHR deferredOperation,
        uint32_t infoCount,
        const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
        const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos)*/


    result = vkCreateAccelerationStructureKHR(device, &tlasCreateInfo, nullptr, &topLevelAS);
    checkIfVkResultIsCorrect(result, "Failed to create empty acceleration structure");
}

