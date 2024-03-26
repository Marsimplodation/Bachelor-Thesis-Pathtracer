#ifndef RAY_H
#define RAY_H
#include "vector.h"
#include "../common.h"
struct Ray {
    Vector3 origin;
    Vector3 direction;
    float throughPut;
    int depth;
    u32 randomState;
    
    //hit info
    Vector3 normal;
    float length;
    bool terminated;
    bool hit;

    //shaderinfo
    void * shaderInfo;
    Vector3 colorMask;
};
Vector3 randomV3UnitHemisphere(Ray & r);
#endif // !RAY_H
