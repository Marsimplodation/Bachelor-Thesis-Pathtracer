#include "bvh.h"
#include "common.h"
#include "scene/scene.h"
#include "shader/shader.h"
#include "types/vector.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <primitives/primitive.h>
#include <utility>
#include <vector>

namespace {
std::vector<BvhNode> nodes;
std::vector<AABB> boxes;
std::vector<int> indices;
std::vector<PrimitiveCompare> splits;
BvhSettings settings{
    .maxDepth = 8, 
};
}

void findBVHIntesection(Ray &ray, int nodeIdx) {
    if(nodeIdx < 0 || nodeIdx >= nodes.size()) return;
    if(ray.terminated) return;
    BvhNode & node = nodes.at(nodeIdx);
    bool leaf = (node.childLeft == -1 && node.childRight == -1);
    
    if(node.AABBIdx >= boxes.size()) return;
    ray.interSectionTests++;
    if(!findIntersection(ray, boxes[node.AABBIdx])) return; 
    findBVHIntesection(ray, node.childLeft);
    findBVHIntesection(ray, node.childRight);
    

    if (!leaf) return;
    for (int i=node.startIdx; i < node.endIdx; i++) {
        if (i >= indices.size()) continue;
        int idx = indices.at(i);
        ray.interSectionTests++;
        findIntersection(ray, idx);
    }
}

BvhSettings *getBvhSettings() {return &settings;}

//----- BVH Structure -----//
void destroyBVH() {
    indices.clear();
    boxes.clear();
    nodes.clear();
}
void calculateBoundingBox(BvhNode & node){
    Vector3 min, max{};
    Vector3 tmin{INFINITY, INFINITY, INFINITY};
    Vector3 tmax{-INFINITY, -INFINITY, -INFINITY};
    void * primitive;
    for (int i= node.startIdx; i < node.endIdx; i++) {
        int idx = indices.at(i);
        primitive = getPrimitive(idx);
        if(!primitive) return;
        tmin = minBounds(primitive);
        tmax = maxBounds(primitive);
        if (tmin.x < min.x) min.x = tmin.x;
        if (tmin.y < min.y) min.y = tmin.y;
        if (tmin.z < min.z) min.z = tmin.z;
        if (tmax.x > max.x) max.x = tmax.x;
        if (tmax.y > max.y) max.y = tmax.y;
        if (tmax.z > max.z) max.z = tmax.z;
    }
    
    // Calculate the center and size of the bounding box
    Vector3 center = {
        (min.x + max.x) * 0.5f,
        (min.y + max.y) * 0.5f,
        (min.z + max.z) * 0.5f
    };
    Vector3 size = {
        max.x - min.x,
        max.y - min.y,
        max.z - min.z
    };
    
    node.AABBIdx = boxes.size();
    boxes.push_back({center, size});
}

int constructBVH(int startIdx, int endIdx, int nodeIdx) {
    bool isRoot = (nodeIdx == -1);
    bool isObject = (startIdx < 0 && endIdx < 0);
    //handle root construction
    if(isRoot) {
        nodeIdx = nodes.size();
        int sIdx = indices.size();
        if(isObject) {
            for(int idx = startIdx; idx > endIdx; idx--) {indices.push_back(idx);}
        }
        else for(int idx = startIdx; idx < endIdx; idx++) {indices.push_back(idx);}
        int eIdx = indices.size();
        nodes.push_back(BvhNode{
            .startIdx = sIdx,
            .endIdx = eIdx,
            .depth = 0,
        });
    }
    
    if(nodeIdx >= nodes.size()) return -1;
    BvhNode & node = nodes.at(nodeIdx);
    calculateBoundingBox(node); 
    //chose split axis
    int splitAxis = std::rand() % 3; 
   //hanlde leaf
    if((node.endIdx - node.startIdx) < 2 || node.depth >= settings.maxDepth) {
        node.childLeft = -1;
        node.childRight = -1;
        node.splitAxis = splitAxis;
        return nodeIdx;
    }
    //get all minValues at splitAxis
    splits.clear();
    for (int i= node.startIdx; i < node.endIdx; i++) {
        int idx = indices.at(i);
        void * primitive = getPrimitive(idx);
        if(!primitive) continue;
        float val = minBounds(primitive)[splitAxis]; 
        splits.push_back({.idx = idx,.val = val});
    }
    //reorder indices
    std::sort(splits.begin(), splits.end());
    for (int i= node.startIdx; i < node.endIdx; i++) {
        indices[i] = splits[i - node.startIdx].idx;
    }
    int halfSize = splits.size() / 2;
    
    BvhNode childLeft {
        .startIdx = node.startIdx,
        .endIdx = node.startIdx + halfSize,
        .depth = node.depth + 1,
    };

    BvhNode childRight {
        .startIdx = node.startIdx + halfSize,
        .endIdx = node.endIdx,
        .depth = node.depth + 1,
    };
    
    node.childLeft = nodes.size();
    node.childRight = nodes.size() + 1;
    nodes.push_back(childLeft);
    nodes.push_back(childRight);
    node = nodes.at(nodeIdx);
    node.childLeft = constructBVH(0, 0, node.childLeft);
    node.childRight = constructBVH(0, 0, node.childRight);
    if(isRoot) {
        printf("build bvh %d\n", nodeIdx);
    }
    return nodeIdx;
}
