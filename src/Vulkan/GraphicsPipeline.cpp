#include "VkRenderer.h"
#include <cstring>
#include <fstream>
#include <vulkan/vulkan_core.h>

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        printf("failed to open file! %s", filename.c_str());
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

VkShaderModule VkRenderer::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
    checkIfVkResultIsCorrect(result, "Failed to create shader module");
    return shaderModule;
}



void VkRenderer::createRaytracingPipeline() {
    auto rgenShaderCode = readFile("Shaders/raygen.spv");
    auto closestHitShaderCode = readFile("Shaders/closesthit.spv");
    auto missShaderCode = readFile("Shaders/miss.spv");

    rgenShaderModule = createShaderModule(rgenShaderCode);
    closesthitShaderModule = createShaderModule(closestHitShaderCode);
    missShaderModule = createShaderModule(missShaderCode);

    VkPipelineShaderStageCreateInfo rgenShaderStageInfo{};
    rgenShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    rgenShaderStageInfo.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    rgenShaderStageInfo.module = rgenShaderModule;
    rgenShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo closesthitShaderStageInfo{};
    closesthitShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    closesthitShaderStageInfo.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    closesthitShaderStageInfo.module = closesthitShaderModule;
    closesthitShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo missShaderStageInfo{};
    missShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    missShaderStageInfo.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
    missShaderStageInfo.module = missShaderModule;
    missShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {rgenShaderStageInfo,
                                                        missShaderStageInfo,
                                                        closesthitShaderStageInfo};



    
    // Create Ray Tracing Shader Groups
    VkRayTracingShaderGroupCreateInfoKHR raygenGroup = {};
    raygenGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    raygenGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    raygenGroup.generalShader = 0;  // Raygen Shader Index
    raygenGroup.closestHitShader = VK_SHADER_UNUSED_KHR;  // Closest Hit Shader Index
    raygenGroup.anyHitShader = VK_SHADER_UNUSED_KHR;  // Make sure to set these to unused
    raygenGroup.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingShaderGroupCreateInfoKHR closestHitGroup = {};
    closestHitGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    closestHitGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    closestHitGroup.closestHitShader = 2;  // Closest Hit Shader Index
    closestHitGroup.anyHitShader = VK_SHADER_UNUSED_KHR;  // Make sure to set these to unused
    closestHitGroup.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingShaderGroupCreateInfoKHR missGroup = {};
    missGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    missGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    missGroup.generalShader = 1;  // Miss Shader Index
    missGroup.closestHitShader = VK_SHADER_UNUSED_KHR;  // Closest Hit Shader Index
    missGroup.anyHitShader = VK_SHADER_UNUSED_KHR;  // Make sure to set these to unused
    missGroup.intersectionShader = VK_SHADER_UNUSED_KHR;

    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups = {raygenGroup, missGroup, closestHitGroup};
    createDescriptorLayout();


    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = descriptorSets.size();
    allocInfo.pSetLayouts = descriptorSetLayouts.data();

    VkResult result = vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data());
    checkIfVkResultIsCorrect(result, "Failed to allocate descriptor set");



    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // You will add descriptor set layouts for ray tracing resources (acceleration structures, buffers, etc.)
    pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size(); // Add descriptor set layouts here if needed
    pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();

    result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    checkIfVkResultIsCorrect(result, "Failed to create pipeline layout");

    // Create the Ray Tracing Pipeline
    VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfo = {};
    rayTracingPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    rayTracingPipelineCreateInfo.stageCount = 3;  // Number of stages (Ray Gen, Miss, Closest Hit)
    rayTracingPipelineCreateInfo.pStages = shaderStages;  // Shader stages array
    rayTracingPipelineCreateInfo.groupCount = 3;
    rayTracingPipelineCreateInfo.pGroups = shaderGroups.data();
    rayTracingPipelineCreateInfo.maxPipelineRayRecursionDepth = 2;  // Maximum recursion depth
    rayTracingPipelineCreateInfo.layout = pipelineLayout;

    result = vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCreateInfo, nullptr, &rtPipeline);  
    checkIfVkResultIsCorrect(result, "Failed to create raytracing pipeline");


    result = CreateSBTBuffers(shaderGroupHandleSize, shaderGroupHandleSize, shaderGroupHandleSize,
                                       &raygenBuffer, &raygenMemory,
                                       &missBuffer, &missMemory,
                                       &hitBuffer, &hitMemory);

    std::vector<uint8_t> shaderHandleStorage(shaderGroupHandleSize * 3);
    vkGetRayTracingShaderGroupHandlesKHR(device, rtPipeline, 0, 3, shaderHandleStorage.size(), shaderHandleStorage.data());


    copyDataToBufferWithStaging(raygenBuffer, shaderHandleStorage.data(), shaderGroupHandleSize);
    copyDataToBufferWithStaging(missBuffer, shaderHandleStorage.data() + 1* shaderGroupHandleSize, shaderGroupHandleSize);
    copyDataToBufferWithStaging(hitBuffer, shaderHandleStorage.data() + 2* shaderGroupHandleSize, shaderGroupHandleSize);

    // Define Ray Tracing Shader Binding Table (SBT)
    
    raygenSBT.deviceAddress = getBufferAdress(raygenBuffer);
    raygenSBT.stride = shaderGroupHandleSize;
    raygenSBT.size = shaderGroupHandleSize;  

    missSBT.deviceAddress = getBufferAdress(missBuffer); 
    missSBT.size = shaderGroupHandleSize;  
    missSBT.stride = shaderGroupHandleSize;

    hitSBT.deviceAddress = getBufferAdress(hitBuffer); 
    hitSBT.size = shaderGroupHandleSize;
    hitSBT.stride = shaderGroupHandleSize;


    
    vkDestroyShaderModule(device, rgenShaderModule, nullptr);
    vkDestroyShaderModule(device, closesthitShaderModule, nullptr);
    vkDestroyShaderModule(device, missShaderModule, nullptr);




}

void VkRenderer::createRenderPasses(){
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;


    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
    


    waveFront = std::vector<RayState>(swapChainExtent.width * swapChainExtent.height);
    for(auto & state : waveFront) {
        state.terminated = true;
    }
    CreateBuffer(sizeof(RayState) * waveFront.size(),
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &waveFrontBuffer,
                 &waveFrontBufferMemory);

    //copyDataToBuffer(waveFrontBufferMemory, waveFront.data(), sizeof(RayState)*waveFront.size());
}
