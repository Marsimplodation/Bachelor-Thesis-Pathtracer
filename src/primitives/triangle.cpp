#include "triangle.h"
#include <cmath>

bool findIntersection(Ray &ray, Triangle & primitive) {
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
    if (t < 0.1 || ray.length < t)
        return false;

    ray.normal = primitive.normal;
    // calculate the tangent and bitangent vectors as well
    // Set the new length and the current primitive
    ray.length = t;
    ray.shaderInfo = primitive.shaderInfo;
    ray.hit = true;
    return true;
}

Triangle createTriangle(Vector3 v0, Vector3 v1, Vector3 v2, void * shaderInfo) {
    Triangle t{
        .vertices = {v0, v1, v2},
        .shaderInfo = shaderInfo,
    };
    t.normal = normalized(crossProduct(t.vertices[1] - t.vertices[0],
                     t.vertices[2] - t.vertices[0]));
    return t;
}
