#include "VkRenderer.h"
void VkRenderer::updateCamera(glm::vec2 mouse, glm::vec2 control, float deltaTime) {
    camera.rotate(mouse, deltaTime);
    camera.move(control, deltaTime);
    copyDataToBuffer(cameraBufferMemory, &camera, sizeof(Camera));
    // Ensure the buffer operation has completed (synchronize)
    VkResult result = vkQueueWaitIdle(graphicsQueue);
    checkIfVkResultIsCorrect(result, "Failed to sync camera buffer");
}
void VkRenderer::createCamera() {
    camera = Camera();
    CreateBuffer(sizeof(Camera) * vertices.size(),
                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                 &cameraBuffer,
                 &cameraBufferMemory, true);
    updateCamera({0,0}, {0,0}, 0.0f);
}

