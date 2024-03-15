#ifndef TRIANGLE_H
#define TRIANGLE_H
#include "../types/vector.h"
#include "../types/ray.h"
struct Triangle {
    Vector3 vertices[3];
    Vector3 normal;
    char shaderFlag;
    void *shaderInfo;
};

bool triangleIntersect(Ray &ray, Triangle & primitive);
#endif // !TRIANGLE_H
