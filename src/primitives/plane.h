#ifndef PLANE_H
#define PLANE_H
#include "../types/ray.h"
#include "../types/vector.h"

struct Plane {
    Vector3 center;
    Vector3 normal;
    void *shaderInfo;
};

bool findIntersection(Ray &ray, Plane & primitive);
Plane createPlane(Vector3 center, Vector3 normal, void *shaderInfo);

#endif // !PLANE_H

