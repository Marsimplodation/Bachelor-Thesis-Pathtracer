#version 460
#extension GL_EXT_ray_tracing : require
#include "include/types.h"
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

layout(set = 1, binding = 0, std430) restrict readonly buffer VertexBuffer {
    Vertex vertices[];
};
layout(set = 1, binding = 1, std430) restrict readonly buffer IndexBuffer {
    uint indices[];
};

hitAttributeEXT vec2 attribs;  // Input: barycentric coordinates
void main() {

    // Get primitive ID
    uint primitiveID = gl_PrimitiveID;

    // Fetch indices for the triangle
    uint index0 = indices[primitiveID * 3 + 0];
    uint index1 = indices[primitiveID * 3 + 1];
    uint index2 = indices[primitiveID * 3 + 2];

    // Fetch vertices from your vertex buffer (similarly as discussed earlier)
    Vertex v0 = vertices[index0];
    Vertex v1 = vertices[index1];
    Vertex v2 = vertices[index2];


    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    vec4 interpolatedColor = v0.color * barycentrics.x + v1.color * barycentrics.y + v2.color * barycentrics.z;


    vec3 origin = gl_WorldRayOriginEXT;      // Ray origin in world space
    vec3 direction = gl_WorldRayDirectionEXT; // Ray direction in world space
    float t = gl_HitTEXT;                      // Distance to the hit point
    vec3 hitPosition = origin + t * direction; // Compute world-space hit position

    rayPayload.hitColor = interpolatedColor.rgb; 
}
