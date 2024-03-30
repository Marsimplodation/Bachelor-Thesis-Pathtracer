#ifndef RAY_H
#define RAY_H
#include "vector.h"
#include "../common.h"
struct Ray {
    Vector3 origin;
    Vector3 direction;
    Vector3 throughPut;
    Vector3 normal;
    
    int depth;
    int interSectionTests;
    u32 randomState;
    
    //hit info
    float length; //gives hit as well
    bool terminated;

    //shaderinfo
    void * shaderInfo;
};
Vector3 randomV3UnitHemisphere(Ray & r);
#endif // !RAY_H
