#version 460
#extension GL_EXT_ray_tracing : require
#include "include/types.h"
#include "include/random.h"
#include "include/layout_rmiss.h"
#include "include/sampling.h"

bool hitVolume(vec3 origin, vec3 direction, vec4 normal) {
    float density = camera.fogDensity; 
    float xi1 = fastRandom(waveFront[rayPayload.idx].randomState);
    float xi2 = fastRandom(waveFront[rayPayload.idx].randomState);
    
    float t = -log(1-xi1)/density;
    //if(t > gl_HitTEXT) return false;
    
    vec3 hitPosition = origin + (t-EPS) * direction; // Compute world-space hit position
    //do the volume Shading
    waveFront[rayPayload.idx].throughPut.rgb *= exp(-t * density);
    
    if(camera.emissiveTriangleCount != 0) {
        NEE(hitPosition, normal.xyz, true);
    }
    waveFront[rayPayload.idx].terminated = true;
    return true;
}
void main() {
    if(rayPayload.isShadowRay == false) {
        vec3 origin = gl_WorldRayOriginEXT;      // Ray origin in world space
        vec3 direction = gl_WorldRayDirectionEXT; // Ray direction in world space
        hitVolume(origin, direction, vec4(0.0));
    }
    rayPayload.hitDistance = INFINITY; 
    waveFront[rayPayload.idx].terminated = true;
}

