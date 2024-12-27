#include "VkRenderer.h"
#include "glm/ext/matrix_float3x3.hpp"
#include "glm/trigonometric.hpp"
#include <cstdio>
void VkRenderer::updateCamera(glm::vec2 mouse, glm::vec2 control, float deltaTime) {
    camera.rotate(mouse, deltaTime);
    camera.move(control, deltaTime);
    camera.lightCount = emissiveTriangles.size();
    copyDataToBuffer(cameraBufferMemory, &camera, sizeof(Camera));
    // Ensure the buffer operation has completed (synchronize)
    VkResult result = vkQueueWaitIdle(graphicsQueue);
    checkIfVkResultIsCorrect(result, "Failed to sync camera buffer");
}
void VkRenderer::createCamera() {
    camera = Camera();
    CreateBuffer(sizeof(Camera) * vertices.size(),
                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT| VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &cameraBuffer,
                 &cameraBufferMemory, true);
    updateCamera({0,0}, {0,0}, 0.0f);
}

//--- Camera struct ---//
Camera::Camera() {
    yaw = 0.0f;
    pitch = 0.0f;
    forward = {0,0,-1,1};
    position = {0,0,5,1};
    up = {0,1,0,1};
    right = {1,0,0,1};
    rotate({0, 0}, 1.0f/20.0f);
    setNewFOV(70);
    reset = true;
    lightCount = 0;
}

void Camera::setNewFOV(float deg) {
    fov = (tanf((deg * 3.14f / 180.0f) / 2.0f));
}

glm::mat3 create_rotation_matrix(float yaw, float pitch, float roll) {
    // Convert degrees to radians
    float cy = cos(glm::radians(yaw));   // cos(yaw)
    float sy = sin(glm::radians(yaw));   // sin(yaw)
    float cp = cos(glm::radians(pitch)); // cos(pitch)
    float sp = sin(glm::radians(pitch)); // sin(pitch)
    float cr = cos(glm::radians(roll));  // cos(roll)
    float sr = sin(glm::radians(roll));  // sin(roll)

    // Compute the rotation matrix in the order of roll (R), pitch (P), yaw (Y)
    glm::mat3 dest=glm::mat3();
    dest[0][0] = cp * cr;
    dest[0][1] = cp * sr;
    dest[0][2] = -sp;

    dest[1][0] = sy * sp * cr - cy * sr;
    dest[1][1] = sy * sp * sr + cy * cr;
    dest[1][2] = sy * cp;

    dest[2][0] = cy * sp * cr + sy * sr;
    dest[2][1] = cy * sp * sr - sy * cr;
    dest[2][2] = cy * cp;
    return dest;
}

void Camera::rotate(glm::vec2 mouse, float deltaTime) {

    const float sensitivity = 20.0f*deltaTime;
    float xoffset = mouse[0];
    float yoffset = mouse[1];
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // Update camera forward
    yaw += yoffset;
    pitch += xoffset;
    if (yaw > 90.0f) yaw = 90.0f;
    if (yaw < -90.0f) yaw = -90.0f;
    if (pitch > 360.0f) pitch = 0.0f;
    if (pitch < -360.0f) pitch = 0.0f;

    glm::mat3 rotation = create_rotation_matrix(yaw, pitch, 0.0f);
    //right handed coordinate system
    glm::vec3 f = {0,0,-1};
    glm::vec3 u = {0,1,0};
    glm::vec3 r = {1,0,0};

    glm::vec3 fw = rotation * f;
    forward.x = fw.x;
    forward.y = fw.y; 
    forward.z = fw.z; 
    auto uw = rotation * u;
    up.x = uw.x;
    up.y = uw.y; 
    up.z = uw.z; 
    auto rw = rotation * r;
    right.x = rw.x;
    right.y = rw.y; 
    right.z = rw.z; 
    //printf("%f %f %f\n", up.x, up.y, up.z);
    //up = rotation * u;
    //right = rotation * r;
}

void Camera::move(glm::vec2 control, float deltaTime) {
    control *= 10.0f * deltaTime;
    position += control.x * forward
                + control.y * right;
}
