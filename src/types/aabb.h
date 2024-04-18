#ifndef AABB_H
#define AABB_H

#include "types/ray.h"
#include "types/vector.h"
struct AABB{
    Vector3 center;
    Vector3 size;
};
bool findIntersection(Ray & ray, AABB & primitive);
Vector3 minBounds(AABB &primitive);
Vector3 maxBounds(AABB &primitive);
void loadPoints(AABB & primitive, Vector3 * points); 

#endif // !AABB_H
