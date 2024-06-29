#include "types/aabb.h"
#include "types/vector.h"
#include <cmath>
#include <utility>

Vector3 minBounds(AABB &primitive){
    return {
        primitive.center.x - primitive.size.x / 2.0f -0.1f,
        primitive.center.y - primitive.size.y / 2.0f -0.1f,
        primitive.center.z - primitive.size.z / 2.0f -0.1f,
    };
}

Vector3 maxBounds(AABB &primitive){
    return {
        primitive.center.x + primitive.size.x / 2.0f +0.1f,
        primitive.center.y + primitive.size.y / 2.0f +0.1f,
        primitive.center.z + primitive.size.z / 2.0f +0.1f,
    };
}

bool findIntersection(Ray &ray, AABB & primitive) {
    Vector3 const minBound = minBounds(primitive);
    Vector3 const maxBound = maxBounds(primitive);
    float tNear = -INFINITY;
    float tFar = +INFINITY;
    //pixars aabb test
    for (int a = 0; a < 3; a++) {
            auto invD = 1 / ray.direction[a];
            auto orig = ray.origin[a];

            auto t0 = (minBound[a] - orig) * invD;
            auto t1 = (maxBound[a] - orig) * invD;

            if (invD < 0)
                std::swap(t0, t1);

            if (t0 > tNear) tNear = t0;
            if (t1 < tFar) tFar = t1;

            if (tNear > tFar)
                return false;
        }
        if(tNear > ray.length)  return  false;  
    return true;
}
