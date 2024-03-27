#ifndef BVH_H
#define BVH_H
#include "ray.h"
#include "primitives/cube.h"
#include "scene/scene.h"
#include "types/aabb.h"
#include "vector.h"

struct BvhNode {
    PrimitivesContainer<int> indices;
    int splitAxis; 
    AABB box;
    BvhNode * childLeft;
    BvhNode * childRight;
};

void destroyBVH(BvhNode * node);
void calculateBoundingBox(BvhNode & node);
void constructBVH(BvhNode & node, bool isObject = false);
BvhNode constructBVH(int startIdx, int endIdx, bool isObject = false);
void findBVHIntesection(Ray & ray, BvhNode * node, bool isObject = false);
#endif // !BVH_H
