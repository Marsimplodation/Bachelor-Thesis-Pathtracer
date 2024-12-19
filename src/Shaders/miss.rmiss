#version 460
#extension GL_EXT_ray_tracing : require

#include "include/types.h"
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

void main() {
    rayPayload.hitDistance = 0.0;
    rayPayload.hitColor = vec3(0.1, 0.1, 0.1); // Example: set miss color to blue
}

