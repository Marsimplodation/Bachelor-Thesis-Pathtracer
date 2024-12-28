#include "types.h"
layout(location = 0) rayPayloadInEXT RayPayload rayPayload;
hitAttributeEXT vec2 attribs;  // Input: barycentric coordinates

//---- Geometry ----//
layout(set = 1, binding = 0, std430) restrict readonly buffer VertexBuffer {
    Vertex vertices[];
};

layout(set = 1, binding = 1, std430) restrict readonly buffer IndexBuffer {
    uint indices[];
};

layout(set = 1, binding = 2, std430) restrict readonly buffer EmissiveBuffer {
    uint emissive_trianles[];
};

layout(set = 0, binding = 6, std430)buffer WaveFrontBuffer {
    RayState waveFront[];
};
layout(set = 0, binding = 4) uniform accelerationStructureEXT topLevelAS;

//---- Materials ----//

layout(set = 2, binding = 0, std430) restrict readonly buffer MaterialBuffer {
    Material materials[];
};
layout(set = 2, binding = 1) buffer TextureBuffer {
    vec4 pixels[];
};
//--- Camera for rendering Settings --//
layout(set = 0, binding = 5) uniform CameraBlock {
    vec4 position;  // Camera position
    vec4 forward; // Camera direction (where it's looking)
    vec4 right; // Camera direction (where it's looking)
    vec4 up;        // Up vector
    float fov;          // Field of view
    float yaw;
    float pitch;
    bool reset;
    uint emissiveTriangleCount;
    bool volumetricFog; 
    float fogDensity;
    float fogScale;
} camera;

layout(set = 1, binding = 3) buffer ObjectBlocks {
    Object objects[];
};

