#include "camera.h"
#include "Core"
#include "common.h"
#include "ray.h"
#include "window/window.h"
#include <cmath>

namespace {
Camera camera{
    .origin= {2.0f, 0.0f, -500.0f},
    .forward= {0, 0, 1},
    .right= {-1, 0, 0},
    .up= {0, 1, 0},
    .focus= 1.0f,
    .fov = 70,
    .fovScale =  (tanf((70 * 3.14f / 180.0f) / 2.0f)),
    .dof = 0,
    .speed = 300.0f,
    .sensitivity = 50.0f,
};
}

Camera *getCamera() { return &camera; }

void cameraSetForward(Vector3 &v) {
    camera.forward = v;
    Vector3 buff[3];
    orthoNormalized(camera.forward, camera.up, camera.right, buff);
    camera.forward = buff[0];
    camera.up = buff[1];
    camera.right = buff[2];
}

void cameraSetUp(Vector3 &v) {
    camera.up = v;
    Vector3 buff[3];
    orthoNormalized(camera.forward, camera.up, camera.right, buff);
    camera.forward = buff[0];
    camera.up = buff[1];
    camera.right = buff[2];
}

void cameraSetFov(float f) {
    camera.fovScale = (tanf((f * 3.14f / 180.0f) / 2.0f));
}

void createCameraRay(float x, float y, Ray &ray) {
    ray.origin = camera.origin;
    ray.direction = normalized(camera.right * (x * camera.fovScale) +
                               camera.up * (y * camera.fovScale) +
                               camera.forward);
    float xi1 = fastRandom(ray.randomState) * camera.dof;
    float xi2 = fastRandom(ray.randomState) * camera.dof;
    Vector3 lensRandom = camera.right * xi1 + camera.up * xi2;  
    ray.origin += lensRandom;
    ray.direction = normalized(ray.direction * camera.focus - lensRandom);


    ray.inv_dir[0] = 1.0f/ray.direction[0];
    ray.inv_dir[1] = 1.0f/ray.direction[1];
    ray.inv_dir[2] = 1.0f/ray.direction[2];
    ray.tmax = INFINITY;
    ray.tmin = 0.0f;
    ray.throughPut = {1,1,1};
    ray.terminated = false;
}



void create_rotation_matrix(float yaw, float pitch, float roll, Eigen::Matrix3f &dest) {
    // Convert degrees to radians
    float cy = std::cos(yaw * M_PI / 180.0f);   // cos(yaw)
    float sy = std::sin(yaw * M_PI / 180.0f);   // sin(yaw)
    float cp = std::cos(pitch * M_PI / 180.0f); // cos(pitch)
    float sp = std::sin(pitch * M_PI / 180.0f); // sin(pitch)
    float cr = std::cos(roll * M_PI / 180.0f);  // cos(roll)
    float sr = std::sin(roll * M_PI / 180.0f);  // sin(roll)

    // Compute the rotation matrix in the order of roll (R), pitch (P), yaw (Y)
    dest(0, 0) = cp * cr;
    dest(1, 0) = cp * sr;
    dest(2, 0) = -sp;

    dest(0, 1) = sy * sp * cr - cy * sr;
    dest(1, 1) = sy * sp * sr + cy * cr;
    dest(2, 1) = sy * cp;

    dest(0, 2) = cy * sp * cr + sy * sr;
    dest(1, 2) = cy * sp * sr - sy * cr;
    dest(2, 2) = cy * cp;
}

void move_camera(Vector2 keyboard,float delta) {
    if(keyboard[0] == 0.0f && keyboard[1] == 0.0f) return;
    camera.origin[0] += keyboard[0] * camera.forward[0] * camera.speed * delta;
    camera.origin[1] += keyboard[0] * camera.forward[1] * camera.speed * delta;
    camera.origin[2] += keyboard[0] * camera.forward[2] * camera.speed * delta;
    camera.origin[0] += keyboard[1] * camera.right[0] * camera.speed * delta;
    camera.origin[1] += keyboard[1] * camera.right[1] * camera.speed * delta;
    camera.origin[2] += keyboard[1] * camera.right[2] * camera.speed * delta;
    callReset();
}


float lastYaw = 0.0f;
float lastPitch = 0.0f;
void rotate_camera(Vector2 mouse, float delta) {
    const float sensitivity = camera.sensitivity*delta;
    float xoffset = mouse[0];
    float yoffset = mouse[1];
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // Update camera forward
    camera.yaw += yoffset;
    camera.pitch += xoffset;
    if (camera.yaw > 90.0f) camera.yaw = 90.0f;
    if (camera.yaw < -90.0f) camera.yaw = -90.0f;
    if (camera.pitch > 360.0f) camera.pitch = 0.0f;
    if (camera.pitch < -360.0f) camera.pitch = 0.0f;
    float yaw = camera.yaw;
    float pitch = camera.pitch;


    Eigen::Matrix3f rotation;
    create_rotation_matrix(yaw, pitch, 0.0f, rotation);
    Eigen::Vector3f f = {0,0,1};
    Eigen::Vector3f u = {0,1,0};
    Eigen::Vector3f r = {-1,0,0};
    // Perform matrix-vector multiplications
    camera.forward.vec = rotation * f;
    camera.up.vec = rotation * u;
    camera.right.vec = rotation * r;
    callReset();
}
