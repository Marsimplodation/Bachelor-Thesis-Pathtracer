#ifndef BVH_H
#define BVH_H
#include "ray.h"
#include "scene/scene.h"
#include "types/aabb.h"
#include "vector.h"
#include <cmath>
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
    float cost = INFINITY;
    bool hasTris;
};

BvhSettings *getBvhSettings();
void destroyBVH();
void calculateBoundingBox(BvhNode &node, bool isObject = false);
int constructBVH(int startIdx, int endIdx, int nodeIdx = -1, bool isObject = false);
bool findBVHIntesection(Ray &ray, int nodeIdx);
#endif // !BVH_H
