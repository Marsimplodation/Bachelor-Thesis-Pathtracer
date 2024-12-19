#ifndef TYPES_H
#define TYPES_H
struct RayPayload {
    vec3 hitColor;
    float hitDistance;
};

struct Vertex {
    vec4 position;
    vec4 normal;
    vec4 color;
};
#define EPS 0.00001
#endif
