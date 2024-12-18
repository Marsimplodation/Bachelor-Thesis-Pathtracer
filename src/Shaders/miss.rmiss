#version 460
#extension GL_EXT_ray_tracing : require

struct RayPayload {
    vec3 hitColor;
    float hitDistance;
};
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;

void main() {
    rayPayload.hitColor = vec3(0.1, 0.1, 0.1); // Example: set miss color to blue
}

