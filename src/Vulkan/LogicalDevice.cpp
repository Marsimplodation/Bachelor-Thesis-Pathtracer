#include "VkRenderer.h"
#include <iostream>
#include <ostream>
#include <set>
#include <vulkan/vulkan_core.h>

void VkRenderer::createLogicalDevice() {
    auto indices = findQueueFamilies(physicalDevice);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<u32> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePrio = 1.0f;
    for (u32 queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePrio;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    
    // Check for ray tracing and acceleration structure support
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingFeatures{};
    rayTracingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
    accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    
    VkPhysicalDeviceAccelerationStructureFeaturesKHR bufferDeviceAddressFeatures{};
     bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;

    // Chain both feature structs
    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures2.pNext = &rayTracingFeatures;  // First, link ray tracing features
    rayTracingFeatures.pNext = &accelerationStructureFeatures;  // Then link acceleration structure features
    accelerationStructureFeatures.pNext = &bufferDeviceAddressFeatures;

    // Query the physical device for these features
    vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);

    // Check if ray tracing and acceleration structure are supported
    if (!rayTracingFeatures.rayTracingPipeline) {
        std::cerr << "Ray tracing is not supported on this device, even though the extension is available!" << std::endl;
        return;
    }

    if (!accelerationStructureFeatures.accelerationStructure) {
        std::cerr << "Acceleration structure is not supported on this device, even though the extension is available!" << std::endl;
        return;
    }
    //queery some shaderSpecific info
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtProperties = {};
    rtProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
    VkPhysicalDeviceProperties2 deviceProperties = {};
    deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProperties.pNext = &rtProperties;
    vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties);
    // Access the properties
    shaderGroupBaseAlignment = rtProperties.shaderGroupBaseAlignment;
    shaderGroupHandleSize = rtProperties.shaderGroupHandleSize;


    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = uniqueQueueFamilies.size();
    createInfo.pEnabledFeatures = nullptr;
    createInfo.pNext = &deviceFeatures2;
    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    
    //compatiblity reasons, no longer needed
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
    auto result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
    checkIfVkResultIsCorrect(result, "failed to create logical deivce");
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    

    

    //load raytracing extension
    vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(
        vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR")
    );
    vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
        vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR")
    );
    vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR >(
        vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR")
    );
    vkCmdTraceRaysKHR  = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(
        vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR")
    );

    vkDestroyAccelerationStructureKHR  = reinterpret_cast< PFN_vkDestroyAccelerationStructureKHR >(
        vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR")
    );

   vkGetBufferDeviceAddressKHR   = reinterpret_cast< PFN_vkGetBufferDeviceAddressKHR>(
        vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR")
    );
    vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(
        vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR")
    );

    vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
        vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR")
    );


}
