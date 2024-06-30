#ifndef BVH_H
#define BVH_H
#include "ray.h"
#include "scene/scene.h"
#include "types/aabb.h"
#include "vector.h"
#include <vector>

struct BvhSettings {
    int maxDepth;
};

struct BvhNode {
    int startIdx;
    int endIdx;
    int splitAxis;
    int depth;
    int AABBIdx;
    int childLeft;
    int childRight;
};

BvhSettings *getBvhSettings();
void destroyBVH();
void calculateBoundingBox(BvhNode &node, bool isObject = false);
int constructBVH(int startIdx, int endIdx, int nodeIdx = -1, bool isObject = false);
void findBVHIntesection(Ray &ray, int nodeIdx, bool isObject = false);
#endif // !BVH_H
