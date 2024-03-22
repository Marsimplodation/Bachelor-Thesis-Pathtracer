#ifndef RAY_H
#define RAY_H
#include "vector.h"
struct Ray {
    Vector3 origin;
    Vector3 direction;
    Vector3 color;
    float throughPut;
    int depth;
    
    //hit info
    Vector3 normal;
    float length;
    bool terminated;
    bool hit;

    //shaderinfo
    void * shaderInfo;
    Vector3 colorMask;
};
#endif // !RAY_H
