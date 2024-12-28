#include "types.h"
layout(binding = 3, rgba8) uniform image2D storageImage;
layout(set = 0, binding = 4) uniform accelerationStructureEXT topLevelAS;
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
layout(set = 0, binding = 6, std430)buffer WaveFrontBuffer {
    RayState waveFront[];
};
layout(location = 0) rayPayloadEXT RayPayload rayPayload;


