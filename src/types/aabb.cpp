#include "types/aabb.h"
#include "types/vector.h"
#include <cmath>
#include <utility>

Vector3 minBounds(AABB &primitive){
    return {
        primitive.center.x - primitive.size.x / 2.0f,
        primitive.center.y - primitive.size.y / 2.0f,
        primitive.center.z - primitive.size.z / 2.0f,
    };
}

Vector3 maxBounds(AABB &primitive){
    return {
        primitive.center.x + primitive.size.x / 2.0f,
        primitive.center.y + primitive.size.y / 2.0f,
        primitive.center.z + primitive.size.z / 2.0f,
    };
}

void loadPoints(AABB & primitive, Vector3 * points) {
    if(!points) return;
    if(sizeof(points)/sizeof(points[0]) != 8) return;
    
    points[0] = {
        primitive.center.x - primitive.size.x / 2.0f,
        primitive.center.y - primitive.size.y / 2.0f,
        primitive.center.z - primitive.size.z / 2.0f,
    };

    points[1] = {
        primitive.center.x + primitive.size.x / 2.0f,
        primitive.center.y - primitive.size.y / 2.0f,
        primitive.center.z - primitive.size.z / 2.0f,
    };

    points[2] = {
        primitive.center.x - primitive.size.x / 2.0f,
        primitive.center.y + primitive.size.y / 2.0f,
        primitive.center.z - primitive.size.z / 2.0f,
    };

    points[3] = {
        primitive.center.x - primitive.size.x / 2.0f,
        primitive.center.y - primitive.size.y / 2.0f,
        primitive.center.z + primitive.size.z / 2.0f,
    };

    points[4] = {
        primitive.center.x + primitive.size.x / 2.0f,
        primitive.center.y + primitive.size.y / 2.0f,
        primitive.center.z - primitive.size.z / 2.0f,
    };

    points[5] = {
        primitive.center.x - primitive.size.x / 2.0f,
        primitive.center.y + primitive.size.y / 2.0f,
        primitive.center.z + primitive.size.z / 2.0f,
    };

    points[6] = {
        primitive.center.x + primitive.size.x / 2.0f,
        primitive.center.y - primitive.size.y / 2.0f,
        primitive.center.z + primitive.size.z / 2.0f,
    };

    points[7] = {
        primitive.center.x + primitive.size.x / 2.0f,
        primitive.center.y + primitive.size.y / 2.0f,
        primitive.center.z + primitive.size.z / 2.0f,
    };

}

bool findIntersection(Ray &ray, AABB & primitive) {
    Vector3 const minBound = minBounds(primitive);
    Vector3 const maxBound = maxBounds(primitive);
    float tNear = -INFINITY;
    float tFar = +INFINITY;
    //pixars aabb test
    for (int a = 0; a < 3; a++) {
            auto invD = 1 / getIndex(ray.direction, a);
            auto orig = getIndex(ray.origin, a);

            auto t0 = (getIndex(minBound, a) - orig) * invD;
            auto t1 = (getIndex(maxBound, a) - orig) * invD;

            if (invD < 0)
                std::swap(t0, t1);

            if (t0 > tNear) tNear = t0;
            if (t1 < tFar) tFar = t1;

            if (tNear > tFar)
                return false;
        }
        return true;
}
