#include "camera.h"
#include "ray.h"
#include <cmath>

namespace {
Camera camera{
    {-2.5f, 2.5f, -10},
    {0, 0, 1},
    {0, 1, 0},
    {1, 0, 0},
    1.0f / (tanf((70 * 3.14f / 180.0f) / 2.0f)),
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


void createCameraRay(float x, float y, Ray &ray) {
    ray.origin = camera.origin;
    ray.direction = normalized(camera.right * x + camera.up * y +
                               camera.forward * camera.focus);
    ray.length = MAXFLOAT;
    ray.throughPut = 1.0f;
    ray.hit = false;
    ray.terminated = false;
}
