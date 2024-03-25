#ifndef SPHERE_H
#include "../types/ray.h"
#include "../types/vector.h"
#define SPHERE 0x03
struct Sphere {
    char type;
    Vector3 center;
    float radius;
    void *shaderInfo;
};

bool findIntersection(Ray & ray, Sphere & primitive);
Sphere createSphere(Vector3 center, float radius, void *shaderInfo);
Vector3 minBounds(Sphere &primitive);
Vector3 maxBounds(Sphere &primitive);

#endif // !SPHERE_H
