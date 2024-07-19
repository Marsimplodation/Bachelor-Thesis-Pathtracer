#include "bvh.h"
#include "common.h"
#include "primitives/object.h"
#include "primitives/triangle.h"
#include "scene/scene.h"
#include "shader/shader.h"
#include "types/aabb.h"
#include "types/vector.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <primitives/primitive.h>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {
std::vector<BvhNode> nodes;
std::vector<AABB> boxes;
std::vector<u32> indicies;

struct Bin {
    std::vector<u32> indicies;
};
std::vector<Bin> bins(2);
} // namespace

bool findBVHIntesection(Ray &ray, int nodeIdx, bool isObject) {
    if (nodeIdx < 0 || nodeIdx >= nodes.size())
        return false;
    if (ray.terminated)
        return false;
    auto &objectBuffer = getObjects();
    auto &indicieBuffer = getIndicies();
    auto &trisBuffer = getTris();

    BvhNode &node = nodes.at(nodeIdx);
    bool leaf = (node.childLeft == -1 && node.childRight == -1);

    if (node.AABBIdx >= boxes.size())
        return false;
    ray.interSectionTests++;
    if (!findIntersection(ray, boxes[node.AABBIdx]))
        return false;
   
    if(!leaf) {
        if(node.childLeft < 0 && node.childRight >= 0) {
            return findBVHIntesection(ray, node.childRight, isObject);
        }else if(node.childLeft >= 0 && node.childRight < 0) {
            return findBVHIntesection(ray, node.childLeft, isObject);
        } else {
            bool hit = false;
            auto & left = nodes[node.childLeft];
            auto & right = nodes[node.childRight];
            auto minLeft = minBounds(boxes[left.AABBIdx])[node.splitAxis];
            auto minRight = minBounds(boxes[right.AABBIdx])[node.splitAxis];
            float dir = ray.direction[node.splitAxis];
            
            int first = node.childLeft;
            int second = node.childRight;

            hit |= findBVHIntesection(ray, first, isObject);
            hit |= findBVHIntesection(ray, second, isObject);
            return hit;    
        }
        return false;
    }

    bool hit = false;
    for (int i = node.startIdx; i < node.endIdx; i++) {
        if (i >= indicies.size())
            continue;
        int idx = indicies[i];
        ray.interSectionTests++;
        if (isObject)
            hit |= triangleIntersection(ray, trisBuffer[idx]);
        else
            hit |= objectIntersection(ray, objectBuffer[idx]);
    }
    return hit;
}

//----- BVH Structure -----//
void destroyBVH() {
    indicies.clear();
    boxes.clear();
    nodes.clear();
}

float evaluateSplit(bool isObject) {
    Vector3 min1 = {INFINITY, INFINITY, INFINITY};
    Vector3 max1 = {-INFINITY, -INFINITY, -INFINITY};
    Vector3 min2 = {INFINITY, INFINITY, INFINITY};
    Vector3 max2 = {-INFINITY, -INFINITY, -INFINITY};
    Vector3 min3 = {INFINITY, INFINITY, INFINITY};
    Vector3 max3 = {-INFINITY, -INFINITY, -INFINITY};

    auto &objectBuffer = getObjects();
    auto &indicieBuffer = getIndicies();
    auto &trisBuffer = getTris();
    u32 primitiveCount1 = 0;
    u32 primitiveCount2 = 0;

    // left node
    for (auto idx : bins[0].indicies) {
        Vector3 pmax = isObject ? maxBounds(trisBuffer[idx])
                                : maxBounds(objectBuffer[idx]);
        max1.x = std::fmaxf(max1.x, pmax.x);
        max1.y = std::fmaxf(max1.y, pmax.y);
        max1.z = std::fmaxf(max1.z, pmax.z);
        Vector3 pmin = isObject ? minBounds(trisBuffer[idx])
                                : minBounds(objectBuffer[idx]);
        min1.x = std::fminf(min1.x, pmin.x);
        min1.y = std::fminf(min1.y, pmin.y);
        min1.z = std::fminf(min1.z, pmin.z);
        primitiveCount1++;
    }

    // right node
    for (auto idx : bins[1].indicies) {
        Vector3 pmax = isObject ? maxBounds(trisBuffer[idx])
                                : maxBounds(objectBuffer[idx]);
        max2.x = std::fmaxf(max2.x, pmax.x);
        max2.y = std::fmaxf(max2.y, pmax.y);
        max2.z = std::fmaxf(max2.z, pmax.z);
        Vector3 pmin = isObject ? minBounds(trisBuffer[idx])
                                : minBounds(objectBuffer[idx]);
        min2.x = std::fminf(min2.x, pmin.x);
        min2.y = std::fminf(min2.y, pmin.y);
        min2.z = std::fminf(min2.z, pmin.z);
        primitiveCount2++;
    }

    // parent node
    for (int i = 0; i < 2; i++) {
        for (auto idx : bins[i].indicies) {
            Vector3 pmax = isObject ? maxBounds(trisBuffer[idx])
                                    : maxBounds(objectBuffer[idx]);
            max3.x = std::fmaxf(max3.x, pmax.x);
            max3.y = std::fmaxf(max3.y, pmax.y);
            max3.z = std::fmaxf(max3.z, pmax.z);
            Vector3 pmin = isObject ? minBounds(trisBuffer[idx])
                                    : minBounds(objectBuffer[idx]);
            min3.x = std::fminf(min3.x, pmin.x);
            min3.y = std::fminf(min3.y, pmin.y);
            min3.z = std::fminf(min3.z, pmin.z);
        }
    }
    
    Vector3 extent1 = max1 - min1;
    Vector3 extent2 = max2 - min2;
    Vector3 extent3 = max3 - min3;
    float area1 = extent1.x * (extent1.y + extent1.x) + extent1.y * extent1.z;
    float area2 = extent2.x * (extent2.y + extent2.x) + extent2.y * extent2.z;
    float area3 = extent3.x * (extent3.y + extent3.x) + extent3.y * extent3.z;
    float cTrav = 1.0f;
    float cInter = 1.0f;
    //this should not be necessary
    //if(primitiveCount1 * primitiveCount2 == 0) return -INFINITY;

    return cTrav + (area1/area3) * cInter * primitiveCount1 +
           (area2/area3) * cInter * primitiveCount2;
}

void calculateBoundingBox(BvhNode &node, bool isObject) {
    auto &objectBuffer = getObjects();
    auto &indicieBuffer = getIndicies();
    auto &trisBuffer = getTris();
    Vector3 min, max{};
    Vector3 tmin{INFINITY, INFINITY, INFINITY};
    Vector3 tmax{-INFINITY, -INFINITY, -INFINITY};
    void *primitive;
    for (int i = node.startIdx; i < node.endIdx; i++) {
        int idx = indicies.at(i);
        tmin = isObject ? minBounds(trisBuffer[idx])
                        : minBounds(objectBuffer[idx]);
        tmax = isObject ? maxBounds(trisBuffer[idx])
                        : maxBounds(objectBuffer[idx]);
        if (tmin.x < min.x)
            min.x = tmin.x;
        if (tmin.y < min.y)
            min.y = tmin.y;
        if (tmin.z < min.z)
            min.z = tmin.z;
        if (tmax.x > max.x)
            max.x = tmax.x;
        if (tmax.y > max.y)
            max.y = tmax.y;
        if (tmax.z > max.z)
            max.z = tmax.z;
    }

    // Calculate the center and size of the bounding box
    Vector3 center = {(min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f,
                      (min.z + max.z) * 0.5f};
    Vector3 size = {max.x - min.x, max.y - min.y, max.z - min.z};

    node.AABBIdx = boxes.size();
    boxes.push_back({center, size});
}

int constructBVH(int startIdx, int endIdx, int nodeIdx, bool isObject) {
    auto &objectBuffer = getObjects();
    auto &indicieBuffer = getIndicies();
    auto &trisBuffer = getTris();

    bool isRoot = (nodeIdx == -1);
    // handle root construction
    if (isRoot) {
        nodeIdx = nodes.size();
        int sIdx = indicies.size();
        for (int idx = startIdx; idx < endIdx; idx++) {
            indicies.push_back(idx);
        }
        int eIdx = indicies.size();
        nodes.push_back(BvhNode{
            .startIdx = sIdx,
            .endIdx = eIdx,
            .depth = 0,
            .cost = INFINITY,
        });
    }

    if (nodeIdx >= nodes.size())
        return -1;
    BvhNode &node = nodes.at(nodeIdx);
    calculateBoundingBox(node, isObject);
    // hanlde leaf
    // chose split axis
    int bestAxis = rand() % 3;
    int bestSplit = 6;
    float bestSAHScore = node.cost;
    auto nodeAABB = boxes[node.AABBIdx];
    bool split = false;
    auto nodeMax = maxBounds(nodeAABB);
    auto nodeMin = minBounds(nodeAABB);
    #define SPLITS 12
    for (int axis = 0; axis < 3; ++axis) {
        // build bins
        float delta = nodeMax[axis] - nodeMin[axis];
        for (int i = 1; i <= SPLITS; ++i) {
            
            float splitPos = nodeMin[axis] + delta * ((float)i)/((float)SPLITS+1.0f);

            bins[0].indicies.resize(0);
            bins[1].indicies.resize(0);
            
            //sort in
            for (int j = node.startIdx; j < node.endIdx; j++) {
                int idx = indicies.at(j);
                Vector3 min;
                if (!isObject) {
                    min = objectBuffer[idx].boundingBox.center;
                } else {
                    min = (minBounds(trisBuffer[idx]) + maxBounds(trisBuffer[idx]))/2.0f;
                }
                
                if(min[axis] < splitPos) {
                    bins[0].indicies.push_back(idx);
                } else {
                    bins[1].indicies.push_back(idx);
                }
            }
            
            //evaluate
            float cost = evaluateSplit(isObject);
            if (cost >= bestSAHScore)
                continue;
            
            bestSAHScore = cost;
            bestAxis = axis;
            bestSplit = i;
            split=true;
        }
    }
    //no split possiblew/more expensive
        node.cost = bestSAHScore;
    if(!split) {
        node.childLeft = -1;
        node.childRight = -1;
        node.splitAxis = 0;
        return nodeIdx;
    }

    std::vector<u32> idxL(0);
    std::vector<u32> idxR(0);
    float delta = nodeMax[bestAxis] - nodeMin[bestAxis];
    float splitPos = nodeMin[bestAxis] + delta * ((float)bestSplit)/((float)SPLITS+1.0f);
    
    //sort in
    for (int j = node.startIdx; j < node.endIdx; j++) {
        int idx = indicies.at(j);
        Vector3 min;
        if (!isObject) {
            min = objectBuffer[idx].boundingBox.center;
        } else {
            min = (minBounds(trisBuffer[idx]) + maxBounds(trisBuffer[idx]))/2.0f;
        }
        
        if(min[bestAxis] < splitPos) {
            idxL.push_back(idx);
        } else {
            idxR.push_back(idx);
        }
    }
    
    if(idxL.size() * idxR.size() == 0) {
        node.childLeft = -1;
        node.childRight = -1;
        node.splitAxis = 0;
        return nodeIdx;
    }
    node.splitAxis = bestAxis;


    int currentIdx = node.startIdx;
    // left node
    for (auto idx : idxL) {
        indicies.at(currentIdx++) = idx;
    }
    int splitIdx = currentIdx;
    for (auto idx : idxR) {
        indicies.at(currentIdx++) = idx;
    }

    BvhNode childLeft{
        .startIdx = node.startIdx,
        .endIdx = splitIdx,
        .depth = node.depth + 1,
    };

    BvhNode childRight{
        .startIdx = splitIdx,
        .endIdx = node.endIdx,
        .depth = node.depth + 1,
    };

    node.childLeft = nodes.size();
    node.childRight = nodes.size() + 1;
    nodes.push_back(childLeft);
    nodes.push_back(childRight);
    node = nodes.at(nodeIdx);
    node.childLeft = constructBVH(0, 0, node.childLeft, isObject);
    node.childRight = constructBVH(0, 0, node.childRight, isObject);
    if (isRoot) {
        printf("build bvh %d\n", nodeIdx);
    }
    return nodeIdx;
}
