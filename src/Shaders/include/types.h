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
    uint shadowRayIndex;
};

struct RayPayload {
    float hitDistance;
    int idx;
};

struct Vertex {
    vec4 position;
    vec4 normal;
    vec2 uv;
    uint materialID;
    uint padding[1];
};

struct Material {
    vec4 color;
    vec4 textureData;
    vec4 normalMapData; //offset, width, height, 
    float emission;
    float ior1;
    float ior2;
    uint shaderFlag;
};

struct Object {
    uint offset;
    uint numTris;
    uint padding[2];
};

#endif
