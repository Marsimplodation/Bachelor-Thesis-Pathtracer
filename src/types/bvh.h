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
void calculateBoundingBox(BvhNode &node);
int constructBVH(int startIdx, int endIdx, int nodeIdx = -1);
void findBVHIntesection(Ray &ray, int nodeIdx);
#endif // !BVH_H
