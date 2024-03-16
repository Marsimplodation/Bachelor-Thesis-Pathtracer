#ifndef SPHERE_H
#include "../types/ray.h"
#include "../types/vector.h"

struct Sphere {
    Vector3 center;
    float radius;
    char shaderFlag;
    void *shaderInfo;
};

bool sphereIntersect(Ray & ray, Sphere & primitive);

#endif // !SPHERE_H
