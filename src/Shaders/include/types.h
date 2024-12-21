#ifndef TYPES_H
#define TYPES_H


struct RayState {
    vec4 origin;
    vec4 direction;
    vec4 throughPut;
    vec4 light;
    uint randomState;
    bool terminated;
    uint samples; 
    uint __padding;
};

struct RayPayload {
    float hitDistance;
    int idx;
};

struct Vertex {
    vec4 position;
    vec4 normal;
    vec4 color;
    uint shaderFlag;
    uint padding[3];
};


#define EPS 0.00001
#endif
