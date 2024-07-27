#ifndef AABB_H
#define AABB_H

#include "types/ray.h"
#include "types/vector.h"
struct AABB{
    Vector3 min;
    Vector3 max;
};

inline Vector3 getCenter(AABB &primitive){
    return (primitive.min + primitive.max)*0.5f;
}

inline Vector3 getSize(AABB &primitive){
    return (primitive.max - primitive.min);
}

bool findIntersection(const Ray &r, const AABB &b);
float getIntersectDistance(const Ray &r, const AABB &b);
#endif // !AABB_H
