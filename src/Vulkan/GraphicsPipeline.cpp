#include "VkRenderer.h"
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


void VkRenderer::createGraphicsPipeline() {
    auto rgenShaderCode = readFile("Shaders/raygen.spv");
    auto closestHitShaderCode = readFile("Shaders/closesthit.spv");
    auto missShaderCode = readFile("Shaders/miss.spv");
    auto rgenShaderModule = createShaderModule(rgenShaderCode);
    auto closesthitShaderModule = createShaderModule(closestHitShaderCode);
    auto missShaderModule = createShaderModule(missShaderCode);

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
                                                        closesthitShaderStageInfo,
                                                        missShaderStageInfo};

    
    // Create Ray Tracing Shader Groups
    VkRayTracingShaderGroupCreateInfoKHR raygenGroup = {};
    raygenGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    raygenGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    raygenGroup.generalShader = 0;  // Raygen Shader Index
    raygenGroup.closestHitShader = VK_SHADER_UNUSED_KHR;  // Closest Hit Shader Index
    raygenGroup.anyHitShader = VK_SHADER_UNUSED_KHR;  // Make sure to set these to unused
    raygenGroup.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingShaderGroupCreateInfoKHR missGroup = {};
    missGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    missGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    missGroup.generalShader = 2;  // Miss Shader Index
    missGroup.closestHitShader = VK_SHADER_UNUSED_KHR;  // Closest Hit Shader Index
    missGroup.anyHitShader = VK_SHADER_UNUSED_KHR;  // Make sure to set these to unused
    missGroup.intersectionShader = VK_SHADER_UNUSED_KHR;

    VkRayTracingShaderGroupCreateInfoKHR closestHitGroup = {};
    closestHitGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    closestHitGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    closestHitGroup.closestHitShader = 1;  // Closest Hit Shader Index
    closestHitGroup.anyHitShader = VK_SHADER_UNUSED_KHR;  // Make sure to set these to unused
    closestHitGroup.intersectionShader = VK_SHADER_UNUSED_KHR;

    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups = {raygenGroup, closestHitGroup, missGroup};
    buildAccelerationStructures();
    



    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // You will add descriptor set layouts for ray tracing resources (acceleration structures, buffers, etc.)
    pipelineLayoutCreateInfo.setLayoutCount = 1; // Add descriptor set layouts here if needed
    pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    checkIfVkResultIsCorrect(result, "Failed to create pipeline layout");
    




    // Create the Ray Tracing Pipeline
    VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfo = {};
    rayTracingPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    rayTracingPipelineCreateInfo.stageCount = 3;  // Number of stages (Ray Gen, Miss, Closest Hit)
    rayTracingPipelineCreateInfo.pStages = shaderStages;  // Shader stages array
    rayTracingPipelineCreateInfo.groupCount = 3;
    rayTracingPipelineCreateInfo.pGroups = shaderGroups.data();
    rayTracingPipelineCreateInfo.maxPipelineRayRecursionDepth = 1;  // Maximum recursion depth
    rayTracingPipelineCreateInfo.layout = pipelineLayout;

    result = vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCreateInfo, nullptr, &rtPipeline);  
    checkIfVkResultIsCorrect(result, "Failed to create raytracing pipeline");


    

    vkDestroyShaderModule(device, rgenShaderModule, nullptr);
    vkDestroyShaderModule(device, closesthitShaderModule, nullptr);
    vkDestroyShaderModule(device, missShaderModule, nullptr);

}
