#ifndef RAY_H
#define RAY_H
#include "vector.h"
#include "../common.h"
#include <atomic>
#include <cmath>
#include <cstdint>

#define PRIMARY_RAY 0x00
#define REFLECTION_RAY 0x01
#define SHADOW_RAY 0x02
#define OTHER 0x03

struct RayVolumeInfo {
    float tmin = INFINITY;
    float tmax = -INFINITY;
    u32 id = UINT32_MAX;
};

struct Ray {
    Vector3 origin;
    Vector3 direction;
    Vector3 inv_dir;
    
    //shader stuff
    Vector3 throughPut;
    Vector3 light;

    Vector3 normal;
    Vector3 tangent;
    Vector3 bitangent;
    Vector2 uv;
    
    int depth;
    int interSectionTests;
    int interSectionAS;
    u32 randomState;
    
    //hit info
    float tmax; //gives hit as well
    float tmin; //gives hit as well
    std::atomic_bool terminated;
    RayVolumeInfo volumeInfo;

    //shaderinfo
    int materialIdx;
    char rayFLAG = PRIMARY_RAY;
};
Vector3 randomCosineWeightedDirection(Ray & r);
Vector2 uniformSampleDisk(Ray & r);
Vector3 randomUniformDirection(Ray & r);
#endif // !RAY_H
