#ifndef TRIANGLE_H
#define TRIANGLE_H
#include "../types/vector.h"
#include "../types/ray.h"
struct Triangle {
    Vector3 vertices[3];
    Vector3 normal[3];
    void *shaderInfo;
};

bool findIntersection(Ray &ray, Triangle & primitive);
Triangle createTriangle(Vector3 v0, Vector3 v1, Vector3 v2, void * shaderInfo);
Triangle createTriangle(Vector3 v0, Vector3 v1, Vector3 v2, Vector3 n0, Vector3 n1, Vector3 n2, void * shaderInfo); 
Vector3 minBounds(Triangle & primitive);
Vector3 maxBounds(Triangle & primitive);
#endif // !TRIANGLE_H
