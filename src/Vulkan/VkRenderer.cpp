#include "VkRenderer.h"
#include "GLFW/glfw3.h"
#include <vulkan/vulkan_core.h>

void VkRenderer::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

//--- PRIVATE ---//

void VkRenderer::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
}

void VkRenderer::initVulkan() {
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createDescriptorPool();
    createRenderPasses();
    createFramebuffers();
    createCommandPool();
    createCommandBuffer();
    loadGeometry();
    createGeometryBuffers();
    buildAccelerationStructures();
    createRaytracingPipeline();
    createSyncObjects();
}

void VkRenderer::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }

}

void VkRenderer::drawFrame(float deltaTime) {
    vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(commandBuffer, imageIndex, deltaTime);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;


    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(presentQueue, &presentInfo);
}

double lxpos, lypos=0.0;
void VkRenderer::handleInput(float deltaTime) {
    double xpos, ypos;
    int windowWidth, windowHeight;
    glfwGetCursorPos(window, &xpos, &ypos);
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glm::vec2 mouse = {0,0};
    glm::vec2 movement = {0,0};
    if(xpos >= 0 && xpos <= windowWidth &&
        ypos >= 0 && ypos <= windowHeight) {
        if(glfwGetMouseButton(window, 1)) {
            mouse.x = xpos - lxpos;
            mouse.y = ypos - lypos;
            xpos = lxpos;
            ypos = lypos;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetCursorPos(window, xpos, ypos);
            camera.reset = true;
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    lxpos = xpos;
    lypos = ypos;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) movement[0] = 1;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) movement[0] = -1;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) movement[1] = -1;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) movement[1] = 1;
    if(movement != glm::vec2{0,0}) camera.reset = true;
    updateCamera(-mouse, movement, deltaTime);
}

void VkRenderer::mainLoop() {
    double lastTime = glfwGetTime();  // Initial time
    createCamera();
    gui = ImguiModule();
    gui.init(device, physicalDevice, instance, graphicsQueue, renderPass, swapChainImages.size(), window);

    bool guiButtonAvailable = true;

    while (!glfwWindowShouldClose(window)) {
        camera.reset = gui.updated;
        gui.updated = false;
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;
        //printf("FPS: %f\n", 1/deltaTime);
        glfwPollEvents();
        handleInput(deltaTime);

        // Keyboard input handling
        if (guiButtonAvailable &&  glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
            guiButtonAvailable = false;
            gui.active = !gui.active;
        }
        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_RELEASE) {
            guiButtonAvailable = true;
        }
        drawFrame(deltaTime);
    }

    vkDeviceWaitIdle(device);
}

void VkRenderer::cleanup() {
    // Wait for the device to finish operations before cleanup
    vkDeviceWaitIdle(device);
    gui.destroy(device);

    // Cleanup synchronization objects
    vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
    vkDestroyFence(device, inFlightFence, nullptr);

    // Cleanup Vulkan objects in reverse order of their creation, considering dependencies
    vkDestroyPipeline(device, rtPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

    vkDestroyAccelerationStructureKHR(device, bottomLevelAS, nullptr);
    vkDestroyAccelerationStructureKHR(device, topLevelAS, nullptr);
        // Free allocated device memory
    //vkFreeMemory(device, topLevelASMemory, nullptr);
    vkFreeMemory(device, imageMemory, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
    vkFreeMemory(device, raygenMemory, nullptr);
    vkFreeMemory(device, missMemory, nullptr);
    vkFreeMemory(device, cameraBufferMemory, nullptr);
    vkFreeMemory(device, materialBufferMemory, nullptr);
    vkFreeMemory(device, textureBufferMemory, nullptr);
    vkFreeMemory(device, waveFrontBufferMemory, nullptr);
    vkFreeMemory(device, hitMemory, nullptr);
    vkFreeMemory(device, topLevelASMemory, nullptr);
    vkFreeMemory(device, instanceASMemory, nullptr);
    vkFreeMemory(device, bottomLevelASMemory, nullptr);

    vkDestroyBuffer(device, hitBuffer, nullptr);
    vkDestroyBuffer(device, missBuffer, nullptr);
    vkDestroyBuffer(device, raygenBuffer, nullptr);
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkDestroyBuffer(device, cameraBuffer, nullptr);
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkDestroyBuffer(device, topLevelASBuffer, nullptr);
    vkDestroyBuffer(device, bottomLevelASBuffer, nullptr);
    vkDestroyBuffer(device, instanceASBuffer, nullptr);
    vkDestroyBuffer(device, materialBuffer, nullptr);
    vkDestroyBuffer(device, textureBuffer, nullptr);
    vkDestroyBuffer(device, waveFrontBuffer, nullptr);

    vkDestroyRenderPass(device, renderPass, nullptr);

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayouts[0], nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayouts[1], nullptr);

    vkDestroyCommandPool(device, commandPool, nullptr);

    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroyImageView(device, storageImageView, nullptr);
    vkDestroyImage(device, storageImage, nullptr);

    vkDestroySwapchainKHR(device, swapChain, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);

    vkDestroyDevice(device, nullptr);

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}



