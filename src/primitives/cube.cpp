#include "cube.h"
#include "types/vector.h"
#include <utility>
#include <cmath>
bool findIntersection(Ray &ray, Cube & primitive) {
    if(!primitive.active) return false;
    Vector3 const minBound = minBounds(primitive);
    Vector3 const maxBound = maxBounds(primitive);
    float tNear = -INFINITY;
    float tFar = +INFINITY;
    int tNearIndex = 0;
    int tFarIndex = 0;
    for (int a = 0; a < 3; a++) {
            auto invD = 1 / getIndex(ray.direction, a);
            auto orig = getIndex(ray.origin, a);

            auto t0 = (getIndex(minBound, a) - orig) * invD;
            auto t1 = (getIndex(maxBound, a) - orig) * invD;

            if (invD < 0)
                std::swap(t0, t1);

            if (t0 > tNear){
                tNear = t0;
                tNearIndex = a;    
            }
            if (t1 < tFar){
                tFar = t1;
                tFarIndex = a;
            }

            if (tNear > tFar)
                return false;
        }

    // Check whether we missed the box completely
    if (tFar < 0 || tNear > tFar)
        return false;

    // Check whether we are on the outside or on the inside of the box
    float const t = (tNear >= 0 ? tNear : tFar);
    int const tIndex = tNear >= 0 ? tNearIndex : tFarIndex;

    // Test whether this is the foremost primitive in front of the camera
    if (ray.length < t)
        return false;
    ray.normal = {0, 0, 0};
    setIndex(ray.normal, tIndex, std::copysignf(1.0f, getIndex(ray.direction, tIndex) * (tNear < 0.0f ? 1.0f : -1.0f)));
    normalize(ray.normal);
    ray.length = t;
    ray.materialIdx = primitive.materialIdx;
    return true;
}
Cube createCube(Vector3 center, Vector3 size, int materialIdx){
    return {
        .type = CUBE,
        .center = center,
        .size = size,
        .active = true,
        .materialIdx = materialIdx,
    };
}

Vector3 minBounds(Cube &primitive){
    return {
        primitive.center.x - primitive.size.x / 2.0f,
        primitive.center.y - primitive.size.y / 2.0f,
        primitive.center.z - primitive.size.z / 2.0f,
    };
}

Vector3 maxBounds(Cube &primitive){
    return {
        primitive.center.x + primitive.size.x / 2.0f,
        primitive.center.y + primitive.size.y / 2.0f,
        primitive.center.z + primitive.size.z / 2.0f,
    };
}
