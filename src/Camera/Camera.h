#ifndef CAMERA_H
#define CAMERA_H
#include "../common.h"

struct alignas(16) Camera {
    glm::vec4 position;  // Camera position
    glm::vec4 forward; // Camera direction (where it's looking)
    glm::vec4 right; // Camera direction (where it's looking)
    glm::vec4 up;        // Up vector
    float fov;          // Field of view
    float yaw;
    float pitch;
    u32 reset;
    void rotate(glm::vec2 mouse, float deltaTime);
    void move(glm::vec2 control, float deltaTime);
    void setNewFOV(float deg);
    Camera();
};
#endif // !CAMERA_H
