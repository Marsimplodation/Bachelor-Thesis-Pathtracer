#include "plane.h"
#include <cmath>

bool findIntersection(Ray &ray, Plane & primitive) {
    // Determine two neighboring edge vectors
    float cosine = dotProduct(ray.direction, primitive.normal); 
    if(cosine > 0) return false;
    
    // Test whether this is the foremost primitive in front of the camera
    float const t = dotProduct(primitive.center - ray.origin, primitive.normal) / cosine;
    if (t < 0.1 || ray.length < t)
        return false;

    ray.normal = primitive.normal;
    // calculate the tangent and bitangent vectors as well
    // Set the new length and the current primitive
    ray.length = t;
    ray.shaderInfo = primitive.shaderInfo;
    return true;
}

Plane createPlane(Vector3 center, Vector3 normal, void *shaderInfo) {
    return {
        .type = PLANE,
        .center = center,
        .normal = normal,
        .shaderInfo = shaderInfo,
    };
}

Vector3 minBounds(Plane &primitive) {
    return {
        primitive.normal.x == 1 ? primitive.center.x : 0,
        primitive.normal.y == 1 ? primitive.center.y : 0,
        primitive.normal.z == 1 ? primitive.center.z : 0,
    };
}

Vector3 maxBounds(Plane &primitive) {
    return {
        primitive.normal.x == 1 ? primitive.center.x : 0,
        primitive.normal.y == 1 ? primitive.center.y : 0,
        primitive.normal.z == 1 ? primitive.center.z : 0,
    };
}
