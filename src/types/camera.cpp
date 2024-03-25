#include "camera.h"
#include "ray.h"
#include <cmath>

namespace {
Camera camera{
    .origin= {2.0f, 0.0f, -9.0f},
    .forward= {0, 0, 1},
    .right= {1, 0, 0},
    .up= {0, 1, 0},
    .focus= 1.0f / (tanf((70 * 3.14f / 180.0f) / 2.0f)),
    .fov = 70,
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
    camera.focus = 1.0f / (tanf((f * 3.14f / 180.0f) / 2.0f));
}

void createCameraRay(float x, float y, Ray &ray) {
    ray.origin = camera.origin;
    ray.direction = normalized(camera.right * x + camera.up * y +
                               camera.forward * camera.focus);
    ray.length = MAXFLOAT;
    ray.throughPut = 1.0f;
    ray.hit = false;
    ray.terminated = false;
}
