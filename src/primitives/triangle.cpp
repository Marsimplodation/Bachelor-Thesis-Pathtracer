#include "triangle.h"
#include "types/vector.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

bool triangleIntersection(Ray &ray, Triangle & primitive) {
    if(!primitive.active) return false;
    // Determine two neighboring edge vectors
    Vector3 const edge1 =
        primitive.vertices[1] - primitive.vertices[0];
    Vector3 const edge2 =
        primitive.vertices[2] - primitive.vertices[0];

    // Begin calculating determinant
    Vector3 const pVec = crossProduct(ray.direction, edge2);

    // Make sure the ray is not parallel to the triangle
    float const det = dotProduct(edge1, pVec);
    if (fabs(det) < 0.01)
        return false;
    float const inv_det = 1.0f / det;

    // Calculate u and test bound
    Vector3 const tVec = ray.origin - primitive.vertices[0];
    float const u = dotProduct(tVec, pVec) * inv_det;
    // Test whether the intersection lies outside the triangle
    if (0.0f > u || u > 1.0f)
        return false;

    // Calculate v and test bound
    Vector3 const qVec = crossProduct(tVec, edge1);
    float const v = dotProduct(ray.direction, qVec) * inv_det;
    // Test whether the intersection lies outside the triangle
    if (0.0f > v || u + v > 1.0f)
        return false;

    // Test whether this is the foremost primitive in front of the camera
    float const t = dotProduct(edge2, qVec) * inv_det;
    if (t < 0.00001f || ray.tmax < t)
        return false;

    ray.normal = normalized(u * primitive.normal[1] + v * primitive.normal[2] + (1 - u - v) * primitive.normal[0]);
    ray.tangent = normalized(crossProduct(ray.normal, abs(ray.normal.z) < 0.999 ? Vector3{0,0,1} : Vector3{1.0, 0.0, 0.0}));
    ray.bitangent = crossProduct(ray.normal, ray.tangent); 
    ray.uv = (u * primitive.uv[1] + v * primitive.uv[2] + (1 - u - v) * primitive.uv[0]);
    // calculate the tangent and bitangent vectors as well
    // Set the new length and the current primitive
    ray.tmax = t;
    ray.materialIdx = primitive.materialIdx;
    return true;
}

Triangle createTriangle(Vector3 v0, Vector3 v1, Vector3 v2, int materialIdx) {
    Triangle t{
        .vertices = {v0, v1, v2},
        .active = true,
        .materialIdx = materialIdx,
    };
    t.normal[0] = normalized(crossProduct(t.vertices[1] - t.vertices[0],
                     t.vertices[2] - t.vertices[0]));
    t.normal[1] = t.normal[0];
    t.normal[2] = t.normal[0];
    return t;
}

Triangle createTriangle(Vector3 v0, Vector3 v1, Vector3 v2, Vector3 n0, Vector3 n1, Vector3 n2, Vector2 uv0, Vector2 uv1, Vector2 uv2, int materialIdx) {
    Triangle t{
        .type = TRIANGLE,
        .vertices = {v0, v1, v2},
        .normal = {n0, n1, n2},
        .uv = {uv0, uv1, uv2},
        .active = true,
        .materialIdx = materialIdx,
    };
    return t;
}

Vector3 minBounds(Triangle &primitive) {
    return {
        std::min(primitive.vertices[0].x, std::min(primitive.vertices[1].x, primitive.vertices[2].x)),
        std::min(primitive.vertices[0].y, std::min(primitive.vertices[1].y, primitive.vertices[2].y)),
        std::min(primitive.vertices[0].z, std::min(primitive.vertices[1].z, primitive.vertices[2].z)),
    };
}

Vector3 maxBounds(Triangle &primitive) {
    return {
        std::max(primitive.vertices[0].x, std::max(primitive.vertices[1].x, primitive.vertices[2].x)),
        std::max(primitive.vertices[0].y, std::max(primitive.vertices[1].y, primitive.vertices[2].y)),
        std::max(primitive.vertices[0].z, std::max(primitive.vertices[1].z, primitive.vertices[2].z)),
    };
}

float calculateTriangleSurfaceArea(Triangle & primitive) {
    auto edge1 = primitive.vertices[2] - primitive.vertices[0];
    auto edge2 = primitive.vertices[1] - primitive.vertices[0];
    auto edge3 = primitive.vertices[2] - primitive.vertices[1];

    float l1 = length(edge1);
    float l2 = length(edge2);
    float l3 = length(edge3);

    float semiPerimeter = (l1 + l2 + l3) / 2;
    float sl1 = semiPerimeter -l1;
    float sl2 = semiPerimeter -l2;
    float sl3 = semiPerimeter -l3;
    float area = sqrtf(semiPerimeter * sl1 * sl2 * sl3);
    return area;
}
