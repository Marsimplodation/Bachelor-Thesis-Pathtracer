#include "camera.h"
#include "common.h"
#include "ray.h"
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
