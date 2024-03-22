#ifndef CUBE_H
#define CUBE_H
#include "../types/ray.h"
#include "../types/vector.h"

struct Cube {
    Vector3 center;
    Vector3 size;
    void *shaderInfo;
};

bool findIntersection(Ray &ray, Cube & primitive);
Cube createCube(Vector3 center, Vector3 size, void *shaderInfo);
Vector3 minBounds(Cube &primitive);
Vector3 maxBounds(Cube &primitive);

#endif // !CUBE_H
