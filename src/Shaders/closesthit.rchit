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

layout(set = 0, binding = 4) uniform accelerationStructureEXT topLevelAS;

hitAttributeEXT vec2 attribs;  // Input: barycentric coordinates

void mirror(vec3 origin, vec3 direction, vec4 normal, vec4 color, float t);
void mirror(vec3 origin, vec3 direction, vec4 normal, vec4 color, float t) {
    vec3 dir = normalize(direction - 2*dot(direction, normal.xyz)*normal.xyz);
    vec3 hitPosition = origin + (t-EPS) * direction; // Compute world-space hit position

    uint flags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT;
    traceRayEXT(topLevelAS,  // acceleration structure
            flags,       // rayFlags
            0xFF,        // cullMask
            0,           // sbtRecordOffset
            0,           // sbtRecordStride
            0,           // missIndex
            hitPosition,      // ray origin
            0.0f,        // ray min range
            dir,      // ray direction
            100.0f,        // ray max range
            0            // payload (location = 1)
    );
    rayPayload.hitColor = color.rgb * rayPayload.hitColor;
}

void lambert(vec3 origin, vec3 direction, vec4 normal, vec4 color, float t);
void lambert(vec3 origin, vec3 direction, vec4 normal, vec4 color, float t) {
    vec3 hitPosition = origin + t * direction; // Compute world-space hit position
    hitPosition += EPS * normal.xyz;
    vec3 lightPos = vec3(0,0.6,0);
    vec3 lightDir = normalize(lightPos - hitPosition);
    float tMax = length(lightPos - hitPosition) - EPS;
    float cos = dot(lightDir, normal.xyz);
    if(cos < 0) {
        rayPayload.hitColor = color.rgb * 0.1;
        return;
    }
    uint flags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;
    rayPayload.hitDistance = tMax;
    traceRayEXT(topLevelAS,  // acceleration structure
            flags,       // rayFlags
            0xFF,        // cullMask
            0,           // sbtRecordOffset
            0,           // sbtRecordStride
            0,           // missIndex
            hitPosition,      // ray origin
            0.0f,        // ray min range
            lightDir,      // ray direction
            tMax,        // ray max range
            0            // payload (location = 1)
    );
    float attenuation = 0.1;
    //rayMiss
    if(rayPayload.hitDistance == 0.0) {
      attenuation = 1*cos;
    }
    rayPayload.hitColor = color.rgb * attenuation;
    rayPayload.hitDistance = t;
}

void main() {
    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    uint primitiveID = gl_PrimitiveID;
    vec3 origin = gl_WorldRayOriginEXT;      // Ray origin in world space
    vec3 direction = gl_WorldRayDirectionEXT; // Ray direction in world space
    float t = gl_HitTEXT;                      // Distance to the hit point

    // Fetch indices for the triangle
    uint index0 = indices[primitiveID * 3 + 0];
    uint index1 = indices[primitiveID * 3 + 1];
    uint index2 = indices[primitiveID * 3 + 2];

    // Fetch vertices from your vertex buffer (similarly as discussed earlier)
    Vertex v0 = vertices[index0];
    Vertex v1 = vertices[index1];
    Vertex v2 = vertices[index2];
    vec4 color = v0.color * barycentrics.x + v1.color * barycentrics.y + v2.color * barycentrics.z;
    vec4 normal = v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z;

    if(v0.shaderFlag == 0x00) {
        lambert(origin, direction,normal, color, t);
    }
    if(v0.shaderFlag == 0x01) {
        mirror(origin, direction, normal, color, t);
    }

}
