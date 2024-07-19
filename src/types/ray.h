#ifndef RAY_H
#define RAY_H
#include "vector.h"
#include "../common.h"
#include <atomic>


struct Ray {
    Vector3 origin;
    Vector3 direction;
    Vector3 inv_dir;
    Vector3 throughPut;
    Vector3 normal;
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
};
Vector3 randomCosineWeightedDirection(Ray & r);
#endif // !RAY_H
