#ifndef OBJECT_H
#define OBJECT_H
#include "../types/ray.h"
#include "../types/vector.h"
#include "cube.h"
#include "types/bvh.h"
#define OBJECT 0x01
struct Object {
    char type;
    const char * name;
    int startIdx;
    int endIdx;
    Cube boundingBox;
    void * shaderInfo;
    BvhNode root;
};

bool findIntersection(Ray & ray, Object & primitive);
void loadObject(const char* fileName, Vector3 position, Vector3 size, void * shaderInfo, void * oBuffer);
Vector3 minBounds(Object &primitive);
Vector3 maxBounds(Object &primitive);

#endif // !OBJECT_H
