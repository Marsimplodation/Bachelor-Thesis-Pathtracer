#include "cube.h"
#include <utility>
#include <cmath>
bool findIntersection(Ray &ray, Cube & primitive) {
    Vector3 const minBound = minBounds(primitive);
    Vector3 const maxBound = maxBounds(primitive);
    auto t1 = (minBound - ray.origin) / ray.direction;
    auto t2 = (maxBound - ray.origin) / ray.direction;

    
    // Determine the intersection points (tNear, tFar)
    // We also have to remember the intersection axes (tNearIndex, tFarIndex)
    float tNear = -MAXFLOAT;
    float tFar = +MAXFLOAT;
    int tNearIndex = 0;
    int tFarIndex = 0;

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
        tNearIndex = 0;
    }
    if (t1.y > tNear) {
        tNear = t1.y;
        tNearIndex = 1;
    }
    if (t1.z > tNear) {
        tNear = t1.z;
        tNearIndex = 2;
    }

    // Check for the far intersection
    if (t2.x < tFar) {
        tFar = t2.x;
        tFarIndex = 0;
    }
    if (t2.y < tFar) {
        tFar = t2.y;
        tFarIndex = 1;
    }
    if (t2.z < tFar) {
        tFar = t2.z;
        tFarIndex = 2;
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
    if (tIndex == 0) ray.normal.x = std::copysignf(1.0f, ray.direction.x * (tNear < 0.0f ? 1.0f : -1.0f)); 
    if (tIndex == 1) ray.normal.y = std::copysignf(1.0f, ray.direction.y * (tNear < 0.0f ? 1.0f : -1.0f)); 
    if (tIndex == 2) ray.normal.z = std::copysignf(1.0f, ray.direction.z * (tNear < 0.0f ? 1.0f : -1.0f));
    normalize(ray.normal);
    ray.length = t;
    ray.shaderInfo = primitive.shaderInfo;
    ray.hit = true;
    return true;
}
Cube createCube(Vector3 center, Vector3 size, void *shaderInfo){
    return {
        .center = center,
        .size = size,
        .shaderInfo = shaderInfo,
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
