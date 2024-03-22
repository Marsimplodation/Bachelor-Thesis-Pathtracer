#ifndef TRIANGLE_H
#define TRIANGLE_H
#include "../types/vector.h"
#include "../types/ray.h"
struct Triangle {
    Vector3 vertices[3];
    Vector3 normal;
    void *shaderInfo;
};

bool findIntersection(Ray &ray, Triangle & primitive);
Triangle createTriangle(Vector3 v0, Vector3 v1, Vector3 v2, void * shaderInfo);
#endif // !TRIANGLE_H
