#include "types/aabb.h"
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

bool findIntersection(Ray &ray, AABB & primitive) {
    Vector3 const minBound = minBounds(primitive);
    Vector3 const maxBound = maxBounds(primitive);
    auto t1 = (minBound - ray.origin) / ray.direction;
    auto t2 = (maxBound - ray.origin) / ray.direction;

    
    // Determine the intersection points (tNear, tFar)
    // We also have to remember the intersection axes (tNearIndex, tFarIndex)
    float tNear = -MAXFLOAT;
    float tFar = +MAXFLOAT;

    // Test the trivial case (and to avoid division by zero errors)
    if ((ray.direction.x == 0 && (ray.origin.x < minBound.x || ray.origin.x > maxBound.x)) ||
        (ray.direction.y == 0 && (ray.origin.y < minBound.y || ray.origin.y > maxBound.y)) ||
        (ray.direction.z == 0 && (ray.origin.z < minBound.z || ray.origin.z > maxBound.z)))
        return false;

    // Swap the bounds if necessary
    if (t1.x > t2.x)
        std::swap(t1.x, t2.x);
    if (t1.y > t2.y)
        std::swap(t1.y, t2.y);
    if (t1.z > t2.z)
        std::swap(t1.z, t2.z);

    // Check for the near intersection
    if (t1.x > tNear) {
        tNear = t1.x;
    }
    if (t1.y > tNear) {
        tNear = t1.y;
    }
    if (t1.z > tNear) {
        tNear = t1.z;
    }

    // Check for the far intersection
    if (t2.x < tFar) {
        tFar = t2.x;
    }
    if (t2.y < tFar) {
        tFar = t2.y;
    }
    if (t2.z < tFar) {
        tFar = t2.z;
    }

    // Check whether we missed the box completely
    if (tFar < 0 || tNear > tFar)
        return false;

    float const t = tNear;    // Test whether this is the foremost primitive in front of the camera
    if (ray.length < t)
        return false;
    return true;
}
