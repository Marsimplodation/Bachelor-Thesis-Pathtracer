#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "../types/ray.h"
#include "../types/vector.h"

struct PointLight {
    Vector3 color;
    Vector3 center;
    float radius;
};

Vector3 illuminate(Ray & r, PointLight & light);

#endif // !POINTLIGHT_H
