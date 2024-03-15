#ifndef PLANE_H
#define PLANE_H
#include "../types/ray.h"
#include "../types/vector.h"

struct Plane {
    Vector3 center;
    Vector3 normal;
    char shaderFlag;
    void *shaderInfo;
};

bool planeIntersect(Ray & ray, Plane & primitive);

#endif // !PLANE_H

