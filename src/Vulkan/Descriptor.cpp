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
    descriptorBindings.push_back({0,0,RGEN,SB});
    descriptorBindings.push_back({0,1,MISS,SB});
    descriptorBindings.push_back({0,2,CHIT,SB});
    descriptorBindings.push_back({0,3,RGEN,IM});
    descriptorBindings.push_back({0,4,ALL_STAGES,AS});
    descriptorBindings.push_back({0,5,ALL_STAGES,UB});
    descriptorBindings.push_back({0,6,ALL_STAGES,SB});
    
    descriptorBindings.push_back({1,0,ALL_STAGES,SB});
    descriptorBindings.push_back({1,1,ALL_STAGES,SB});
    descriptorBindings.push_back({1,2,ALL_STAGES,SB});
    descriptorBindings.push_back({1,3,ALL_STAGES,SB});
    
    descriptorBindings.push_back({2,0,ALL_STAGES,SB});
    descriptorBindings.push_back({2,1,ALL_STAGES,SB});
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
    
    bufferInfos.push_back({.buffer=raygenBuffer});
    bufferInfos.push_back({.buffer=missBuffer});
    bufferInfos.push_back({.buffer=hitBuffer});
    imageInfos.push_back({.imageView = storageImageView});
    asInfos.push_back({.pAccelerationStructures = &topLevelAS});
    imageInfos.push_back({.imageView = storageImageView});
    bufferInfos.push_back({.buffer=cameraBuffer});
    bufferInfos.push_back({.buffer=waveFrontBuffer});

    bufferInfos.push_back({.buffer=vertexBuffer});
    bufferInfos.push_back({.buffer=indexBuffer});
    bufferInfos.push_back({.buffer=emissiveBuffer});
    bufferInfos.push_back({.buffer=objectBuffer});

    bufferInfos.push_back({.buffer=materialBuffer});
    bufferInfos.push_back({.buffer=textureBuffer});


    int asIndex=0;
    int imIndex=0;
    int bIndex=0;
    for (auto & elem : descriptorBindings) {
        VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, nullptr, 0, 0, 0, SB, nullptr, nullptr, nullptr};
        write.dstSet = descriptorSets[elem.set];
        write.descriptorType = elem.type;
        write.dstBinding = elem.binding;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;

        if(elem.type == AS) {
            auto & info = asInfos[asIndex++];
            info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
            info.accelerationStructureCount = 1;
            write.pNext = (void*) &info; 
        }
        else if(elem.type == IM) {
            auto & info = imageInfos[imIndex++];
            info.imageLayout = VK_IMAGE_LAYOUT_GENERAL, // Layout for storage images
            write.pImageInfo = &info; 
        }
        else {
            auto & info = bufferInfos[bIndex++];
            info.offset = 0;
            info.range = VK_WHOLE_SIZE;
            write.pBufferInfo = &info; 
        }
        writeDescriptorSets.push_back(write);
    }
    vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}
