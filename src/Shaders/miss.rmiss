#version 460
#extension GL_EXT_ray_tracing : require

#include "include/types.h"
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;
layout(set = 0, binding = 6, std430)buffer WaveFrontBuffer {
    RayState waveFront[];
};
void main() {
    rayPayload.hitDistance = 0.0;
    waveFront[rayPayload.idx].terminated = true;
}

