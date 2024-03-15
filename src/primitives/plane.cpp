#include "plane.h"

bool planeIntersect(Ray &ray, Plane & primitive) {
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
    ray.shaderFlag = primitive.shaderFlag;
    ray.shaderInfo = primitive.shaderInfo;
    ray.hit = true;
    return true;
}
