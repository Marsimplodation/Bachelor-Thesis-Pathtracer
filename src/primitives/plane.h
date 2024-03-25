#ifndef PLANE_H
#define PLANE_H
#include "../types/ray.h"
#include "../types/vector.h"
#define PLANE 0x02
struct Plane {
    char type;
    Vector3 center;
    Vector3 normal;
    void *shaderInfo;
};

bool findIntersection(Ray &ray, Plane & primitive);
Plane createPlane(Vector3 center, Vector3 normal, void *shaderInfo);
Vector3 minBounds(Plane &primitive);
Vector3 maxBounds(Plane &primitive);
#endif // !PLANE_H

