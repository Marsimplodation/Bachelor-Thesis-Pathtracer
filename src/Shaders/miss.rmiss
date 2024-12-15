#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec3 hitColor;

void main() {
    hitColor = vec3(1.0, 0.0, 1.0); // Example: set miss color to blue
}

