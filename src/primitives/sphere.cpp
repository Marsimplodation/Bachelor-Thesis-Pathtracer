#include "sphere.h"
#include <cmath>
#include <cstdio>

bool findIntersection(Ray &ray, Sphere & primitive){
    Vector3 origin = ray.origin - primitive.center;
    float a = 1.0f;
    float b = 2.0 * dotProduct(origin, ray.direction);
    float c = dotProduct(origin, origin) - primitive.radius * primitive.radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
        return false;

    float const root = std::sqrt(discriminant);

    // Stable solution
    float const q = -0.5f * (b < 0 ? (b - root) : (b + root));
    float const t0 = q / a;
    float const t1 = c / q;
    float t = std::fmin(t0, t1);
    if (t < 0.1)
        t = std::fmax(t0, t1);

    if (t < 0.1 || ray.length < t)
        return false;
    Vector3 const hitPoint = ray.origin + t * ray.direction;
    ray.normal = normalized(hitPoint - primitive.center);
    // calculate the tangent and bitangent vectors as well
    // Set the new length and the current primitive
    ray.length = t;
    ray.materialIdx = primitive.materialIdx;
    return true;
}

Sphere createSphere(Vector3 center, float radius, int materialIdx) {
    return {
        .type = SPHERE,
        .center = center,
        .radius = radius,
        .materialIdx = materialIdx,
    };
}

Vector3 minBounds(Sphere &primitive){
    return {
        primitive.center.x - primitive.radius,
        primitive.center.y - primitive.radius,
        primitive.center.z - primitive.radius,
    };
}

Vector3 maxBounds(Sphere &primitive){
    return {
        primitive.center.x + primitive.radius,
        primitive.center.y + primitive.radius,
        primitive.center.z + primitive.radius,
    };
}
