#ifndef TYPES_H
#define TYPES_H
struct RayPayload {
    vec3 hitColor;
    float hitDistance;
};

struct Vertex {
    vec4 position;
    vec4 color;
};

#endif
