#include "VkRenderer.h"
#include <set>
#include <vector>
#include <vulkan/vulkan_core.h>
namespace {
    #define SB VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    #define UB VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    #define AS VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR
    #define IM VK_DESCRIPTOR_TYPE_STORAGE_IMAGE 

    #define CHIT VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR
    #define RGEN VK_SHADER_STAGE_RAYGEN_BIT_KHR
    #define MISS VK_SHADER_STAGE_MISS_BIT_KHR    
    #define ALL_STAGES VK_SHADER_STAGE_ALL 
}


void VkRenderer::createDescriptorPool() {
    /*struct DescriptorBinding {
        u32 set;
        u32 binding;
        VkShaderStageFlags shaderStage;
        VkDescriptorType type;
        VkBuffer* buffer;
        VkImageView* image;
        VkAccelerationStructureKHR* as;
    };*/
    descriptorBindings = std::vector<DescriptorBinding>();
    descriptorBindings.push_back({0,0,RGEN,SB,&raygenBuffer});
    descriptorBindings.push_back({0,1,MISS,SB,&missBuffer});
    descriptorBindings.push_back({0,2,CHIT,SB,&hitBuffer});
    descriptorBindings.push_back({0,3,RGEN,IM,VK_NULL_HANDLE,&storageImageView});
    descriptorBindings.push_back({0,4,ALL_STAGES,AS,VK_NULL_HANDLE,VK_NULL_HANDLE,&topLevelAS});
    descriptorBindings.push_back({0,5,ALL_STAGES,UB,&cameraBuffer});
    descriptorBindings.push_back({0,6,ALL_STAGES,SB,&waveFrontBuffer});
    
    descriptorBindings.push_back({1,0,ALL_STAGES,SB,&vertexBuffer});
    descriptorBindings.push_back({1,1,ALL_STAGES,SB,&indexBuffer});
    descriptorBindings.push_back({1,2,ALL_STAGES,SB,&emissiveBuffer});
    descriptorBindings.push_back({1,3,ALL_STAGES,SB,&objectBuffer});
    
    descriptorBindings.push_back({2,0,ALL_STAGES,SB,&materialBuffer});
    descriptorBindings.push_back({2,1,ALL_STAGES,SB,&textureBuffer});
    std::vector<VkDescriptorPoolSize> poolSizes(0);
    
    for (auto & elem : descriptorBindings) {
        bool found = false;
        for(auto & poolSize : poolSizes) {
            if(poolSize.type == elem.type) {
                poolSize.descriptorCount++;
                found = true;
            }
        }
        if(!found){
            poolSizes.push_back({.type = elem.type, .descriptorCount = 1});
        }
    }

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 3;  // Only need 1 descriptor set (if you're allocating 1 per frame)

    VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);
    checkIfVkResultIsCorrect(result, "Failed to create descriptor pool");
}

void VkRenderer::createDescriptorLayout() {
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> bindings(3);
    for (auto & elem : descriptorBindings) {
        VkDescriptorSetLayoutBinding binding = {};
        binding.binding = elem.binding;
        binding.descriptorType = elem.type;
        binding.descriptorCount = 1;
        binding.stageFlags = elem.shaderStage;  // This buffer is used by the raygen shader.
        bindings[elem.set].push_back(binding);

    }
    
    for (int i = 0; i < bindings.size(); ++i) {
        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.bindingCount = bindings[i].size();  // Number of bindings
        layoutCreateInfo.pBindings = bindings[i].data();
        VkResult result = vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayouts[i]);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

    }
}



void VkRenderer::updateDescriptorSet() {
    std::vector<VkWriteDescriptorSet> writeDescriptorSets(0);
    std::vector<VkDescriptorBufferInfo> bufferInfos(0);
    std::vector<VkDescriptorImageInfo> imageInfos(0);
    std::vector< VkWriteDescriptorSetAccelerationStructureKHR> asInfos(0);

    for (auto & elem : descriptorBindings) {
        VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, nullptr, 0, 0, 0, SB, nullptr, nullptr, nullptr};
        write.dstSet = descriptorSets[elem.set];
        write.descriptorType = elem.type;
        write.dstBinding = elem.binding;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;

        if(elem.type == AS) {
            int index = asInfos.size();
            asInfos.push_back({
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
                .accelerationStructureCount = 1,
                .pAccelerationStructures = elem.as,
            });
            write.pNext = (void*)(&asInfos[index]); 
        }
        else if(elem.type == IM) {
            int index = imageInfos.size();
            imageInfos.push_back({
                .imageView = *elem.image, // The image view for the storage image
                .imageLayout = VK_IMAGE_LAYOUT_GENERAL, // Layout for storage images
            });
            write.pImageInfo = &imageInfos[index]; 
        }
        else {
            int index = bufferInfos.size();
            bufferInfos.push_back({
                .buffer = *elem.buffer,
                .offset = 0,
                .range = VK_WHOLE_SIZE,
            });
            write.pBufferInfo = &bufferInfos[index]; 
        }
        writeDescriptorSets.push_back(write);
    }
    vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}
