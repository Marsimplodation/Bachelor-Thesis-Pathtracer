#ifndef BVH_H
#define BVH_H
#include "primitives/cube.h"
#include "ray.h"
#include "scene/scene.h"
#include "types/aabb.h"
#include "vector.h"
#include <vector>

struct BvhSettings {
    int maxDepth;
};

struct BvhNode {
    std::vector<int> indices;
    int splitAxis;
    int depth;
    AABB box;
    BvhNode *childLeft = 0x0;
    BvhNode *childRight = 0x0;
};

BvhSettings *getBvhSettings();
void destroyBVH(BvhNode *node);
void calculateBoundingBox(BvhNode &node, bool isObject = false);
void constructBVH(BvhNode &node, bool isObject = false);
BvhNode constructBVH(int startIdx, int endIdx, bool isObject = false);
void findBVHIntesection(Ray &ray, BvhNode *node, bool isObject = false);
#endif // !BVH_H
