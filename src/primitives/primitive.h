#ifndef PRIMITIVE_H
#define PRIMITIVE_H
#include "shader/shader.h"
#include "types/ray.h"
#include "cube.h"
#include "plane.h"
#include "sphere.h"
#include "triangle.h"
#include "object.h"

struct PrimitiveCompare {
    int idx;
    float val;
};

bool operator <(const PrimitiveCompare & p1, const PrimitiveCompare &p2);

bool findIntersection(Ray & ray, int idx);
Vector3 minBounds(void *primitive);
Vector3 maxBounds(void *primitive);
int getMaterial(void* primitive);
#endif // !PRIMITIVE_H
