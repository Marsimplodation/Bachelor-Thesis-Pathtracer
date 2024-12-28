#include "VkRenderer.h"
#include <vulkan/vulkan_core.h>

void VkRenderer::createDescriptorLayout() {
    //----- General Set -----//
    VkDescriptorSetLayoutBinding raygenBinding = {};
    raygenBinding.binding = 0;
    raygenBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    raygenBinding.descriptorCount = 1;
    raygenBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;  // This buffer is used by the raygen shader.


    VkDescriptorSetLayoutBinding hitBinding = {};
    hitBinding.binding = 2;
    hitBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    hitBinding.descriptorCount = 1;
    hitBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;  // This buffer is used by the closest hit shader.
    
    VkDescriptorSetLayoutBinding missBinding = {};
    missBinding.binding = 1;
    missBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    missBinding.descriptorCount = 1;
    missBinding.stageFlags = VK_SHADER_STAGE_MISS_BIT_KHR;  // This buffer is used by the miss shader.

    VkDescriptorSetLayoutBinding storageImageBinding{};
    storageImageBinding.binding = 3; // Binding number in the shader
    storageImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    storageImageBinding.descriptorCount = 1; // Number of storage images (usually 1)
    storageImageBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR; // Which shader stages can access this image
    storageImageBinding.pImmutableSamplers = nullptr; // Not used for storage images
    
    VkDescriptorSetLayoutBinding waveFrontBinding{};
    waveFrontBinding.binding = 6; // Binding number in the shader
    waveFrontBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    waveFrontBinding.descriptorCount = 1; // Number of storage images (usually 1)
    waveFrontBinding.stageFlags = VK_SHADER_STAGE_ALL; // Which shader stages can access this image
    waveFrontBinding.pImmutableSamplers = nullptr; // Not used for storage images
    //
    VkDescriptorSetLayoutBinding asBinding{};
    asBinding.binding = 4;
    asBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    asBinding.descriptorCount = 1;
    asBinding.stageFlags = VK_SHADER_STAGE_ALL;
    asBinding.pImmutableSamplers = nullptr;
    
    VkDescriptorSetLayoutBinding cameraBinding = {};
    cameraBinding.binding = 5;
    cameraBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraBinding.descriptorCount = 1;
    cameraBinding.stageFlags = VK_SHADER_STAGE_ALL;  // This buffer is used by the miss shader.
    VkDescriptorSetLayoutBinding bindings[] = {raygenBinding, missBinding, hitBinding, storageImageBinding, asBinding, cameraBinding, waveFrontBinding};

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = 7;  // Number of bindings
    layoutCreateInfo.pBindings = bindings;
    VkResult result = vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayouts[0]);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
    
    //----- Geometry Set -----//
    VkDescriptorSetLayoutBinding vertexBufferBinding{};
    vertexBufferBinding.binding = 0;
    vertexBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    vertexBufferBinding.descriptorCount = 1;
    vertexBufferBinding.stageFlags = VK_SHADER_STAGE_ALL;
    vertexBufferBinding.pImmutableSamplers = nullptr;
    
    VkDescriptorSetLayoutBinding indexBufferBinding{};
    indexBufferBinding.binding = 1;
    indexBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    indexBufferBinding.descriptorCount = 1;
    indexBufferBinding.stageFlags = VK_SHADER_STAGE_ALL;
    indexBufferBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding emissiveBufferBinding{};
    emissiveBufferBinding.binding = 2;
    emissiveBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    emissiveBufferBinding.descriptorCount = 1;
    emissiveBufferBinding.stageFlags = VK_SHADER_STAGE_ALL;
    emissiveBufferBinding.pImmutableSamplers = nullptr;
    
    VkDescriptorSetLayoutBinding objectBufferBinding{};
    objectBufferBinding.binding = 3;
    objectBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    objectBufferBinding.descriptorCount = 1;
    objectBufferBinding.stageFlags = VK_SHADER_STAGE_ALL;
    objectBufferBinding.pImmutableSamplers = nullptr;


    VkDescriptorSetLayoutBinding bindings_set1[] = {vertexBufferBinding, indexBufferBinding, emissiveBufferBinding, objectBufferBinding};

    // Create the descriptor set layout
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo_set1 = {};
    layoutCreateInfo_set1.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo_set1.bindingCount = 4;  // Number of bindings
    layoutCreateInfo_set1.pBindings = bindings_set1;
    
    result = vkCreateDescriptorSetLayout(device, &layoutCreateInfo_set1, nullptr, &descriptorSetLayouts[1]);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    //----- Material Set -----//
        
    VkDescriptorSetLayoutBinding materialBufferBinding{};
    materialBufferBinding.binding = 0;
    materialBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    materialBufferBinding.descriptorCount = 1;
    materialBufferBinding.stageFlags = VK_SHADER_STAGE_ALL;
    materialBufferBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding textureBufferBinding{};
    textureBufferBinding.binding = 1;
    textureBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    textureBufferBinding.descriptorCount = 1;
    textureBufferBinding.stageFlags = VK_SHADER_STAGE_ALL;
    textureBufferBinding.pImmutableSamplers = nullptr;
    VkDescriptorSetLayoutBinding bindings_set2[] = {materialBufferBinding, textureBufferBinding};
    // Create the descriptor set layout
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo_set2 = {};
    layoutCreateInfo_set2.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo_set2.bindingCount = 2;  // Number of bindings
    layoutCreateInfo_set2.pBindings = bindings_set2;
    
    result = vkCreateDescriptorSetLayout(device, &layoutCreateInfo_set2, nullptr, &descriptorSetLayouts[2]);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

}


void VkRenderer::createDescriptorPool() {
    VkDescriptorPoolSize poolSizes[7] = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = 4; // Change to 3 for the 3 descriptors (raygen, miss, hit)
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[1].descriptorCount = 1; 
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    poolSizes[2].descriptorCount = 1; 
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[3].descriptorCount = 1;
    //set 1 -- geometry
    poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[4].descriptorCount = 4; 
    //set 2 -- materials
    poolSizes[5].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[5].descriptorCount = 2; 

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 6;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 3;  // Only need 1 descriptor set (if you're allocating 1 per frame)

    VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);
    checkIfVkResultIsCorrect(result, "Failed to create descriptor pool");
}

void VkRenderer::updateDescriptorSet() {
    VkDescriptorBufferInfo raygenBufferInfo = {};
    raygenBufferInfo.buffer = raygenBuffer;
    raygenBufferInfo.offset = 0;
    raygenBufferInfo.range = VK_WHOLE_SIZE;

    VkDescriptorBufferInfo missBufferInfo = {};
    missBufferInfo.buffer = missBuffer;
    missBufferInfo.offset = 0;
    missBufferInfo.range = VK_WHOLE_SIZE;

    VkDescriptorBufferInfo hitBufferInfo = {};
    hitBufferInfo.buffer = hitBuffer;
    hitBufferInfo.offset = 0;
    hitBufferInfo.range = VK_WHOLE_SIZE;
    
    VkDescriptorBufferInfo cameraBufferInfo = {};
    cameraBufferInfo.buffer = cameraBuffer;
    cameraBufferInfo.offset = 0;
    cameraBufferInfo.range = VK_WHOLE_SIZE;
    
    VkDescriptorBufferInfo vertexBufferInfo = {};
    vertexBufferInfo.buffer = vertexBuffer;
    vertexBufferInfo.offset = 0;
    vertexBufferInfo.range = VK_WHOLE_SIZE;
    
    VkDescriptorBufferInfo indexBufferInfo = {};
    indexBufferInfo.buffer = indexBuffer;
    indexBufferInfo.offset = 0;
    indexBufferInfo.range = VK_WHOLE_SIZE;
    VkDescriptorBufferInfo emissiveBufferInfo = {};
    emissiveBufferInfo.buffer = emissiveBuffer;
    emissiveBufferInfo.offset = 0;
    emissiveBufferInfo.range = VK_WHOLE_SIZE;
    

    VkDescriptorBufferInfo objectBufferInfo = {};
    objectBufferInfo.buffer = objectBuffer;
    objectBufferInfo.offset = 0;
    objectBufferInfo.range = VK_WHOLE_SIZE;

    VkDescriptorBufferInfo materialBufferInfo = {};
    materialBufferInfo.buffer = materialBuffer;
    materialBufferInfo.offset = 0;
    materialBufferInfo.range = VK_WHOLE_SIZE;
    VkDescriptorBufferInfo textureBufferInfo = {};
    textureBufferInfo.buffer = textureBuffer;
    textureBufferInfo.offset = 0;
    textureBufferInfo.range = VK_WHOLE_SIZE;
    
    VkDescriptorBufferInfo waveFrontBufferInfo = {};
    waveFrontBufferInfo.buffer = waveFrontBuffer;
    waveFrontBufferInfo.offset = 0;
    waveFrontBufferInfo.range = VK_WHOLE_SIZE;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageView = storageImageView; // The image view for the storage image
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // Layout for storage images

    VkWriteDescriptorSetAccelerationStructureKHR accelerationStructureWrite{};
    accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    accelerationStructureWrite.accelerationStructureCount = 1;
    accelerationStructureWrite.pAccelerationStructures = &topLevelAS;

    /*
    typedef struct VkWriteDescriptorSet {
        VkStructureType                  sType;
        const void*                      pNext;
        VkDescriptorSet                  dstSet;
        uint32_t                         dstBinding;
        uint32_t                         dstArrayElement;
        uint32_t                         descriptorCount;
        VkDescriptorType                 descriptorType;
        const VkDescriptorImageInfo*     pImageInfo;
        const VkDescriptorBufferInfo*    pBufferInfo;
        const VkBufferView*              pTexelBufferView;
    } VkWriteDescriptorSet;*/
    VkWriteDescriptorSet writeDescriptorSets[] = {
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[0], 0, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &raygenBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[0], 1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &missBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[0], 2, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &hitBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[0], 3, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &imageInfo, nullptr, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, &accelerationStructureWrite, descriptorSets[0], 4, 0, 1, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, &imageInfo, nullptr, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[0], 5, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &cameraBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[0], 6, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &waveFrontBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[1], 0, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &vertexBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[1], 1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &indexBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[1], 2, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &emissiveBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[1], 3, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &objectBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[2], 0, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &materialBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[2], 1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &textureBufferInfo, nullptr },
        
    };

    vkUpdateDescriptorSets(device, 13, writeDescriptorSets, 0, nullptr);
}
