#include "VkRenderer.h"
#include <vector>
#include <vulkan/vulkan_core.h>
#include <glm/vec3.hpp>

void VkRenderer::buildBottomLevelAS() {
    VkAccelerationStructureGeometryKHR geometry{};
    geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

    // Describe vertex data
    geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    geometry.geometry.triangles.vertexData.deviceAddress = getBufferAdress(vertexBuffer);
    geometry.geometry.triangles.vertexStride = sizeof(Vertex);
    geometry.geometry.triangles.maxVertex = vertices.size(),
    geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;

    // Describe index data
    geometry.geometry.triangles.indexData.deviceAddress = getBufferAdress(indexBuffer);
    geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;

    u32 primitiveCount = indices.size()/3;
    VkAccelerationStructureBuildRangeInfoKHR buildRange{};
    buildRange.primitiveCount = primitiveCount; // Number of triangles
    buildRange.primitiveOffset = 0;
    buildRange.firstVertex = 0;
    buildRange.transformOffset = 0;

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    buildInfo.geometryCount = 1;
    buildInfo.pGeometries = &geometry;

    VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
    sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &primitiveCount, &sizeInfo);
    printf("BLAS size %lu\n", sizeInfo.accelerationStructureSize);
    printf("Scratch size %lu\n", sizeInfo.buildScratchSize);

    
    VkDeviceMemory scratchBufferMemory;
    VkBuffer scratchBuffer;
    CreateBuffer(sizeInfo.buildScratchSize,
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                 &scratchBuffer,
                 &scratchBufferMemory, true);

    buildInfo.scratchData.deviceAddress = getBufferAdress(scratchBuffer);



    CreateBuffer(sizeInfo.accelerationStructureSize,
                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                 &bottomLevelASBuffer,
                 &bottomLevelASMemory, true);

    //--- BOTTOM LEVEL ---//
    VkAccelerationStructureCreateInfoKHR blasCreateInfo = {};
    blasCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    blasCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    blasCreateInfo.size = sizeInfo.accelerationStructureSize;
    blasCreateInfo.buffer = bottomLevelASBuffer;


    VkResult result = vkCreateAccelerationStructureKHR(device, &blasCreateInfo, nullptr, &bottomLevelAS);
    checkIfVkResultIsCorrect(result, "Failed to create empty acceleration structure");
    buildInfo.dstAccelerationStructure = bottomLevelAS;

    beginCommandBuffer();
    const VkAccelerationStructureBuildRangeInfoKHR* pBuildRanges[] = { &buildRange };
    vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildInfo, pBuildRanges);
    endCommandBuffer();
    vkDestroyBuffer(device, scratchBuffer, nullptr);
    vkFreeMemory(device, scratchBufferMemory, nullptr);
}

void VkRenderer::buildTopLevelAS() {
    VkAccelerationStructureDeviceAddressInfoKHR addressInfo = {};
    addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    addressInfo.accelerationStructure = bottomLevelAS;
    VkDeviceAddress blasAdress = vkGetAccelerationStructureDeviceAddressKHR(device, &addressInfo);
    VkAccelerationStructureInstanceKHR instance{};
    //instance.transform = { /* 3x4 row-major transform matrix */ };
    // Transform matrix (row-major 3x4 identity matrix)
    instance.transform.matrix[0][0] = 1.0f;  // Row 0, Column 0
    instance.transform.matrix[0][1] = 0.0f;  // Row 0, Column 1
    instance.transform.matrix[0][2] = 0.0f;  // Row 0, Column 2
    instance.transform.matrix[0][3] = 0.0f;  // Row 0, Column 3 (translation x)

    instance.transform.matrix[1][0] = 0.0f;  // Row 1, Column 0
    instance.transform.matrix[1][1] = 1.0f;  // Row 1, Column 1
    instance.transform.matrix[1][2] = 0.0f;  // Row 1, Column 2
    instance.transform.matrix[1][3] = 0.0f;  // Row 1, Column 3 (translation y)

    instance.transform.matrix[2][0] = 0.0f;  // Row 2, Column 0
    instance.transform.matrix[2][1] = 0.0f;  // Row 2, Column 1
    instance.transform.matrix[2][2] = 1.0f;  // Row 2, Column 2
    instance.transform.matrix[2][3] = 0.0f;  // Row 2, Column 3 (translation z)
    instance.instanceCustomIndex = 0; // Custom ID
    instance.mask = 0xFF;
    instance.instanceShaderBindingTableRecordOffset = 0;
    instance.accelerationStructureReference = blasAdress;


    CreateBuffer(
        sizeof(VkAccelerationStructureInstanceKHR),
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &instanceASBuffer,
        &instanceASMemory,
        true
    );
    copyDataToBuffer(instanceASMemory, &instance, sizeof(VkAccelerationStructureInstanceKHR));


    VkAccelerationStructureGeometryKHR geometry{};
    geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

    geometry.geometry.instances.data.deviceAddress = getBufferAdress(instanceASBuffer);
    geometry.geometry.instances.arrayOfPointers = VK_FALSE;
    geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;


    u32 instanceCount = 1;

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    buildInfo.geometryCount = 1;
    buildInfo.pGeometries = &geometry;

    VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
    sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &instanceCount, &sizeInfo);
    
    printf("TLAS Size %lu\n", sizeInfo.accelerationStructureSize);
    printf("Scratch Size %lu\n", sizeInfo.buildScratchSize);
    VkDeviceMemory scratchBufferMemory;
    VkBuffer scratchBuffer;
    CreateBuffer(sizeInfo.buildScratchSize,
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                 &scratchBuffer,
                 &scratchBufferMemory, true);

    buildInfo.scratchData.deviceAddress = getBufferAdress(scratchBuffer);


    CreateBuffer(sizeInfo.accelerationStructureSize,
                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                 &topLevelASBuffer,
                 &topLevelASMemory, true);


    //--- TOP LEVEL ---//
    VkAccelerationStructureCreateInfoKHR tlasCreateInfo = {};
    tlasCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    tlasCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    tlasCreateInfo.size = sizeInfo.accelerationStructureSize;
    tlasCreateInfo.buffer = topLevelASBuffer;


    VkResult result = vkCreateAccelerationStructureKHR(device, &tlasCreateInfo, nullptr, &topLevelAS);
    checkIfVkResultIsCorrect(result, "Failed to create top level acceleration structure");

    buildInfo.dstAccelerationStructure = topLevelAS;
    VkAccelerationStructureBuildRangeInfoKHR buildRange{};
    buildRange.primitiveCount = instanceCount; // Number of instances
    
    beginCommandBuffer();
    const VkAccelerationStructureBuildRangeInfoKHR* pBuildRanges[] = { &buildRange };
    vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildInfo, pBuildRanges);
    endCommandBuffer();
    vkDestroyBuffer(device, scratchBuffer, nullptr);
    vkFreeMemory(device, scratchBufferMemory, nullptr);
}

void VkRenderer::buildAccelerationStructures() {
    buildBottomLevelAS();
    buildTopLevelAS();
}

