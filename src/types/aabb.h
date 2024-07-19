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
bool cuboidInAABB(AABB & aabb, Vector3 *verts);
bool triInAABB(AABB & aabb, Vector3 *verts);
bool aabbInAABB(AABB & aabbA, AABB & aabbB); 
bool pointInAABB(AABB & aabb, Vector3 v); 

#endif // !AABB_H
