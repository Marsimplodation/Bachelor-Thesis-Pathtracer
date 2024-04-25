#ifndef OBJECT_H
#define OBJECT_H
#include "../types/ray.h"
#include "../types/vector.h"
#include "types/aabb.h"
#include "types/bvh.h"
#include <string>
#define OBJECT 0x01
struct Object {
    char type;
    int startIdx;
    int endIdx;
    AABB boundingBox;
    int materialIdx;
    int root;
    bool active;
    std::string name;
};

bool objectIntersection(Ray & ray, Object & primitive);
void loadObject(const char* fileName, Vector3 position, Vector3 size, int  materialIdx, void * oBuffer);
Vector3 minBounds(Object &primitive);
Vector3 maxBounds(Object &primitive);

#endif // !OBJECT_H
