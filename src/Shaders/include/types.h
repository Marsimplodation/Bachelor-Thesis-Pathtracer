#ifndef TYPES_H
#define TYPES_H


struct RayState {
    vec4 origin;
    vec4 direction;
    vec4 throughPut;
    vec4 light;
    vec4 pixelColor;
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
    uint materialID;
    uint padding[3];
};

struct Material {
    vec4 color;
    float emission;
    float ior1;
    float ior2;
    uint shaderFlag;
};

#endif
