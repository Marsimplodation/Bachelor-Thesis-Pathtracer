#ifndef CUBE_H
#define CUBE_H
#include "../types/ray.h"
#include "../types/vector.h"
#define CUBE 0x00

struct Cube {
    char type;
    Vector3 center;
    Vector3 size;
    bool active;
    int materialIdx;
};

bool findIntersection(Ray &ray, Cube & primitive);
Cube createCube(Vector3 center, Vector3 size, int materialIdx);
Vector3 minBounds(Cube &primitive);
Vector3 maxBounds(Cube &primitive);

#endif // !CUBE_H
