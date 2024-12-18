#include "VkRenderer.h"
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
//--- Descriptor ----//


void VkRenderer::createDescriptorPool() {
    VkDescriptorPoolSize poolSizes[3] = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = 3; // Change to 3 for the 3 descriptors (raygen, miss, hit)
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[1].descriptorCount = 1; 
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    poolSizes[2].descriptorCount = 1; 

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 3;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 1;  // Only need 1 descriptor set (if you're allocating 1 per frame)

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

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageView = storageImageView; // The image view for the storage image
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // Layout for storage images

    VkWriteDescriptorSetAccelerationStructureKHR accelerationStructureWrite{};
    accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    accelerationStructureWrite.accelerationStructureCount = 1;
    accelerationStructureWrite.pAccelerationStructures = &topLevelAS;

    VkWriteDescriptorSet writeDescriptorSets[] = {
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSet, 0, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &raygenBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSet, 1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &missBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSet, 2, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &hitBufferInfo, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSet, 3, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &imageInfo, nullptr, nullptr },
        { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, &accelerationStructureWrite, descriptorSet, 4, 0, 1, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, nullptr, nullptr, nullptr},
    };

    vkUpdateDescriptorSets(device, 5, writeDescriptorSets, 0, nullptr);
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
        1,  // Number of descriptor sets
        &descriptorSet,  // The descriptor set to bind
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
        gui.update(commandBuffer, deltaTime);
    vkCmdEndRenderPass(commandBuffer);


    endCommandBuffer();
}

//------ Geometry ----//
void VkRenderer::createGeometryBuffers() {
    //create AS BUFFERS
    // Triangle vertex data (positions and colors)
    vertices = {
        {{0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Vertex 0
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Vertex 1
        {{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},   // Vertex 2
    };
    indices = {
        0,1,2,
    };
    
    CreateBuffer(sizeof(Vertex) * vertices.size(),
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                 | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                 &vertexBuffer,
                 &vertexBufferMemory, true);
    CreateBuffer(sizeof(u32) * indices.size(),
                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                 | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                 &indexBuffer,
                 &indexBufferMemory, true);


    copyDataToBuffer(vertexBufferMemory, vertices.data(), sizeof(Vertex)*vertices.size());
    copyDataToBuffer(indexBufferMemory, indices.data(), sizeof(u32)*indices.size());
}
