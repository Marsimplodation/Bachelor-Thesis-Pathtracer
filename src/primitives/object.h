#ifndef OBJECT_H
#define OBJECT_H
#include "../types/ray.h"
#include "../types/vector.h"
#include "types/aabb.h"
#include <string>
#include <vector>
#define OBJECT 0x01
struct Object {
    char type;
    int startIdx;
    int endIdx;
    AABB boundingBox;
    int root;
    bool active;
    float surfaceArea;
    u32 GridIdx[3];
    std::string name;
    std::vector<u32> materials;
};

bool objectIntersection(Ray & ray, Object & primitive);
void loadObject(const std::string fileName, Vector3 position, Vector3 size, int  materialIdx);

inline Vector3 minBounds(Object &primitive) {
    return primitive.boundingBox.min;
}
inline Vector3 maxBounds(Object &primitive) {
    return primitive.boundingBox.max;
}

u32 getRandomTriangleFromObject(Ray & ray, Object & primitive);

#endif // !OBJECT_H
