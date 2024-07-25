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
    int childLeft = -1;
    int childRight = -1;
    float cost = INFINITY;
    bool hasTris;
};

BvhSettings *getBvhSettings();
BvhNode & getNode(u32 idx);
AABB & getNodeAABB(u32 idx);
u32 & bvhGetTrisIndex(u32 idx);
inline bool isLeaf(BvhNode & node) {
    return node.childLeft == -1 && node.childRight == -1;
}
void destroyBVH();
void calculateBoundingBox(BvhNode &node, bool isObject = false);
int constructBVH(int startIdx, int endIdx, int nodeIdx = -1, bool isObject = false);
bool findBVHIntesection(Ray &ray, int nodeIdx);
#endif // !BVH_H
