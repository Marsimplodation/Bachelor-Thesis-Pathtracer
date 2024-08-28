#ifndef RAY_H
#define RAY_H
#include "vector.h"
#include "../common.h"
#include <atomic>

#define PRIMARY_RAY 0x00
#define REFLECTION_RAY 0x01
#define OTHER 0x02
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

    //shaderinfo
    int materialIdx;
    char rayFLAG = PRIMARY_RAY;
};
Vector3 randomCosineWeightedDirection(Ray & r);
#endif // !RAY_H
