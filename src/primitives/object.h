#ifndef OBJECT_H
#define OBJECT_H
#include "../types/ray.h"
#include "../types/vector.h"
#include "types/aabb.h"
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
void loadObject(const std::string fileName, Vector3 position, Vector3 size, int  materialIdx);

inline Vector3 minBounds(Object &primitive) {
    return primitive.boundingBox.min;
}
inline Vector3 maxBounds(Object &primitive) {
    return primitive.boundingBox.max;
}

#endif // !OBJECT_H
