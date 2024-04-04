#ifndef BVH_H
#define BVH_H
#include "ray.h"
#include "primitives/cube.h"
#include "scene/scene.h"
#include "types/aabb.h"
#include "vector.h"

struct BvhSettings {
   int maxDepth; 
};

struct BvhNode {
    PrimitivesContainer<int> indices;
    int splitAxis; 
    int depth;
    AABB box;
    BvhNode * childLeft = 0x0;
    BvhNode * childRight = 0x0;
};

BvhSettings * getBvhSettings();
void destroyBVH(BvhNode * node);
void calculateBoundingBox(BvhNode & node, bool isObject = false);
void constructBVH(BvhNode & node, bool isObject = false);
BvhNode constructBVH(int startIdx, int endIdx, bool isObject = false);
void findBVHIntesection(Ray & ray, BvhNode * node, bool isObject = false);
#endif // !BVH_H
