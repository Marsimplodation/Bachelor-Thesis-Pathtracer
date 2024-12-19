#version 460
#extension GL_EXT_ray_tracing : require

struct RayPayload {
    vec3 hitColor;
    float hitDistance;
};
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

void main() {
    vec3 origin = gl_WorldRayOriginEXT;      // Ray origin in world space
    vec3 direction = gl_WorldRayDirectionEXT; // Ray direction in world space
    float t = gl_HitTEXT;                      // Distance to the hit point
    vec3 hitPosition = origin + t * direction; // Compute world-space hit position

    rayPayload.hitColor = hitPosition; 
}
