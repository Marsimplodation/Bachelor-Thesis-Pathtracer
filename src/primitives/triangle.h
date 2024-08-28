#ifndef TRIANGLE_H
#define TRIANGLE_H
#include "../types/vector.h"
#include "../types/ray.h"
#define TRIANGLE 0x04

struct Triangle {
    char type;
    Vector3 vertices[3];
    Vector3 normal[3];
    Vector2 uv[3]; 
    bool active;
    int materialIdx;
};

bool triangleIntersection(Ray &ray, Triangle & primitive);
Triangle createTriangle(Vector3 v0, Vector3 v1, Vector3 v2, int materialIdx);
Triangle createTriangle(Vector3 v0, Vector3 v1, Vector3 v2, Vector3 n0, Vector3 n1, Vector3 n2, Vector2 uv0, Vector2 uv1, Vector2 uv2, int materialIdx);
Vector3 minBounds(Triangle & primitive);
Vector3 maxBounds(Triangle & primitive);
#endif // !TRIANGLE_H
