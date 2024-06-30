#ifndef PRIMITIVE_H
#define PRIMITIVE_H
#include "shader/shader.h"
#include "types/ray.h"
#include "triangle.h"
#include "object.h"

struct PrimitiveCompare {
    int idx;
    float val;
};

bool operator <(const PrimitiveCompare & p1, const PrimitiveCompare &p2);
#endif // !PRIMITIVE_H
