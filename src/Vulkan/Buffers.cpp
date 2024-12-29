#include "VkRenderer.h"
#include "glm/ext/vector_float4.hpp"
#include <cstdio>
#include <vulkan/vulkan_core.h>

//--- INFOS ---//
VkDeviceAddress VkRenderer::getBufferAdress(const VkBuffer & bufferHandle) {
    VkBufferDeviceAddressInfo bufferDeviceAddressInfo{};
    bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
    bufferDeviceAddressInfo.pNext = nullptr; // No additional structures.
    bufferDeviceAddressInfo.buffer = bufferHandle;
    return vkGetBufferDeviceAddressKHR(device, &bufferDeviceAddressInfo);
}

//--- Frame Buffers --- //
void VkRenderer::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}


//--- creating buffers----//
u32 VkRenderer::FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);  // Assuming `physicalDevice` is already initialized

    // Loop through all available memory types and find a suitable one
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        // Check if the type is available in the filter and supports the required properties
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;  // Return the index of the suitable memory type
        }
    }

    // If no suitable memory type is found, we return an invalid index
    throw std::runtime_error("Failed to find suitable memory type!");
}

VkResult VkRenderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                      VkBuffer* buffer, VkDeviceMemory* bufferMemory, bool requiresDeviceAddress) {
    // Create the buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer!");
    }

    // Get memory requirements for the buffer
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

    // Allocate memory
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    // If buffer device address is required, set the VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT flag
    VkMemoryAllocateFlagsInfo allocFlagsInfo{};
    if (requiresDeviceAddress) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        allocInfo.pNext = &allocFlagsInfo;  // Chain the flag info to the allocation info
    }

    if (vkAllocateMemory(device, &allocInfo, nullptr, bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, *buffer, *bufferMemory, 0);

    return VK_SUCCESS;
}


// Function to create SBT buffers
VkResult VkRenderer::CreateSBTBuffers(VkDeviceSize raygenSize, VkDeviceSize missSize, VkDeviceSize hitSize,
                          VkBuffer* raygenBuffer, VkDeviceMemory* raygenMemory,
                          VkBuffer* missBuffer, VkDeviceMemory* missMemory,
                          VkBuffer* hitBuffer, VkDeviceMemory* hitMemory){
    
    // Create the raygen buffer
    VkResult result = CreateBuffer(raygenSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, raygenBuffer, raygenMemory, true);
    if (result != VK_SUCCESS) return result;

    // Create the miss buffer
    result = CreateBuffer(missSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, missBuffer, missMemory, true);
    if (result != VK_SUCCESS) return result;

    // Create the hit buffer
    result = CreateBuffer(hitSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT| VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, hitBuffer, hitMemory, true);
    if (result != VK_SUCCESS) return result;

    return VK_SUCCESS;
}

void VkRenderer::copyDataToBuffer(VkDeviceMemory bufferMemory, const void* shaderCode, VkDeviceSize shaderSize) {
    void* mappedMemory;
    vkMapMemory(device, bufferMemory, 0, shaderSize, 0, &mappedMemory);
    memcpy(mappedMemory, shaderCode, static_cast<size_t>(shaderSize));
    vkUnmapMemory(device, bufferMemory);
}

void VkRenderer::copyDataToBufferWithStaging(VkBuffer buffer, const void* data, VkDeviceSize bufferSize) {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(bufferSize,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     &stagingBuffer, &stagingBufferMemory);

        void* mappedMemory;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &mappedMemory);
        memcpy(mappedMemory, data, static_cast<size_t>(bufferSize));
        vkUnmapMemory(device, stagingBufferMemory);

        //copy data to actual buffer
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = bufferSize;
        vkCmdCopyBuffer(commandBuffer, stagingBuffer, buffer, 1, &copyRegion);
        vkEndCommandBuffer(commandBuffer);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);
        vkDestroyBuffer(device, stagingBuffer, nullptr); 
        vkFreeMemory(device, stagingBufferMemory, nullptr); 
    }

//---- COMMAND BUFFERS ----//
void VkRenderer::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
    checkIfVkResultIsCorrect(result, "Failed to create command pool");
}

void VkRenderer::createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkResult result = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    checkIfVkResultIsCorrect(result, "Failed to create command buffer");
}

void TransitionImageLayout(
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkPipelineStageFlags srcStage,
    VkPipelineStageFlags dstStage) {
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    } else if (newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = 0;
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        srcStage, dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
}

void VkRenderer::endCommandBuffer() {
    // End Command Buffer Recording
    VkResult result = vkEndCommandBuffer(commandBuffer);
    checkIfVkResultIsCorrect(result, "failed to end recording command buffer!");
}
void VkRenderer::beginCommandBuffer() {
    // Begin Command Buffer Recording
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    checkIfVkResultIsCorrect(result, "failed to begin recording command buffer!");
}


void VkRenderer::traceImage() {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = storageImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;  // No need for src access mask when transitioning from UNDEFINED
    barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);
      
    // Bind the Ray Tracing Pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rtPipeline);


    
    updateDescriptorSet();
    vkCmdBindDescriptorSets(
        commandBuffer, 
        VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, 
        pipelineLayout, 
        0,  // Descriptor set binding point
        3,  // Number of descriptor sets
        descriptorSets,  // The descriptor set to bind
        0,  // Dynamic offsets count (if using dynamic descriptors)
        nullptr  // Dynamic offsets (if applicable)
    );
    //printf("Tracing time\n");
    // Dispatch Rays
    vkCmdTraceRaysKHR(
        commandBuffer,
        &raygenSBT,    // Raygen shader binding table
        &missSBT,      // Miss shader binding table
        &hitSBT,       // Hit shader binding table
        &callableSBT,  // Callable shader binding table (optional)
        swapChainExtent.width,  // Ray tracing image width
        swapChainExtent.height, // Ray tracing image height
        1               // Depth (for 2D images, use 1)
    );
}

void VkRenderer::copyTracedImageToSwapchain(int imageIndex) {
    // Transition the storage image for transfer
    TransitionImageLayout(
        commandBuffer,
        storageImage,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT
    );

    // Transition the swapchain image for transfer
    TransitionImageLayout(
        commandBuffer,
        swapChainImages[imageIndex],
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT
    );
    VkImageCopy imageCopyRegion{};
    imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopyRegion.srcSubresource.mipLevel = 0;
    imageCopyRegion.srcSubresource.baseArrayLayer = 0;
    imageCopyRegion.srcSubresource.layerCount = 1;
    imageCopyRegion.srcOffset = { 0, 0, 0 };

    imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopyRegion.dstSubresource.mipLevel = 0;
    imageCopyRegion.dstSubresource.baseArrayLayer = 0;
    imageCopyRegion.dstSubresource.layerCount = 1;
    imageCopyRegion.dstOffset = { 0, 0, 0 };

    imageCopyRegion.extent = {
        swapChainExtent.width,
        swapChainExtent.height,
        1
    };

    vkCmdCopyImage(
        commandBuffer,
        storageImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        swapChainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &imageCopyRegion
    );
    TransitionImageLayout(
        commandBuffer,
        swapChainImages[imageIndex],
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
    );
    TransitionImageLayout(
        commandBuffer,
        storageImage,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
    );
}

void VkRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, float deltaTime) {
    beginCommandBuffer();
    traceImage();
    copyTracedImageToSwapchain(imageIndex);

    //render pass for drawing UI    
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.clearValueCount = 0;
    renderPassInfo.pClearValues = VK_NULL_HANDLE;
    renderPassInfo.renderArea.extent = swapChainExtent; 
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        gui.update(this, deltaTime);
    vkCmdEndRenderPass(commandBuffer);


    endCommandBuffer();
}

//------ Geometry ----//
void VkRenderer::createGeometryBuffers() {
    //create AS BUFFERS
    // Triangle vertex data (positions and colors)
    
// Cube vertex data (positions and colors)
    CreateBuffer(sizeof(Vertex) * vertices.size(),
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                 | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &vertexBuffer,
                 &vertexBufferMemory, true);
    CreateBuffer(sizeof(u32) * indices.size(),
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                 | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &indexBuffer,
                 &indexBufferMemory, true);

    CreateBuffer(sizeof(Material) * materials.size(),
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &materialBuffer,
                 &materialBufferMemory);

    u32 texSize = textureAtlas.size() == 0? 1 : textureAtlas.size();
     CreateBuffer(sizeof(glm::vec4) * texSize,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &textureBuffer,
                 &textureBufferMemory);
    
    //allocate for each triangle to be emissive in the worst case
    CreateBuffer(sizeof(u32) * indices.size() / 3,
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &emissiveBuffer,
                 &emissiveBufferMemory);

    CreateBuffer(sizeof(objects) * objects.size() ,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &objectBuffer,
                 &objectBufferMemory);

    copyDataToBufferWithStaging(vertexBuffer, vertices.data(), sizeof(Vertex)*vertices.size());
    copyDataToBufferWithStaging(indexBuffer, indices.data(), sizeof(u32)*indices.size());
    copyDataToBuffer(materialBufferMemory, materials.data(), sizeof(Material)*materials.size());

    if(textureAtlas.size() > 0)
        copyDataToBufferWithStaging(textureBuffer, textureAtlas.data(), sizeof(glm::vec4)*textureAtlas.size());

    if(emissiveTriangles.size() > 0)
        copyDataToBuffer(emissiveBufferMemory, emissiveTriangles.data(), sizeof(u32)*emissiveTriangles.size());
    
    copyDataToBufferWithStaging(objectBuffer, objects.data(), sizeof(Object)*objects.size());
}
