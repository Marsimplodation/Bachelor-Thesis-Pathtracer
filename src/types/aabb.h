#ifndef AABB_H
#define AABB_H

#include "types/ray.h"
#include "types/vector.h"
struct AABB{
    Vector3 center;
    Vector3 size;
};
bool findIntersection(Ray & ray, AABB & primitive);

#endif // !AABB_H
