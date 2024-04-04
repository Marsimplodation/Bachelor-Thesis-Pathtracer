#include "bvh.h"
#include "common.h"
#include "scene/scene.h"
#include "shader/shader.h"
#include "types/vector.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <primitives/primitive.h>
#include <vector>

namespace {
BvhSettings settings{
    .maxDepth = 4,
};
}

BvhSettings *getBvhSettings() {return &settings;}

//----- BVH Structure -----//
void destroyBVH(BvhNode * node) {
    if(node == 0x0) return;
    destroyBVH(node->childLeft);
    destroyBVH(node->childRight);
    
    destroyContainer(node->indices);
    if(node)delete node;
}
void calculateBoundingBox(BvhNode & node, bool isObject){
    Vector3 min, max{};
    Vector3 tmin{INFINITY, INFINITY, INFINITY};
    Vector3 tmax{-INFINITY, -INFINITY, -INFINITY};
    void * primitive;
    int idx = 0; 
    for (int i = 0; i < node.indices.count; i++) {
        idx = node.indices.data[i];
        if(!isObject) primitive = getPrimitive(idx);
        else primitive = getObjectBufferAtIdx(idx);
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
    
    node.box = {.center = center, .size=size};
}

void findBVHIntesection(Ray & ray, BvhNode * node, bool isObject) {
    if(!node) return;
    if(ray.terminated) return;
    bool leaf = (node->depth == settings.maxDepth) || !node->childLeft && !node->childRight; 
    //check if it can collide in the first place
    float origin = getIndex(ray.origin, node->splitAxis);
    float dir = getIndex(ray.direction, node->splitAxis);
    float max = getIndex(maxBounds(node->box), node->splitAxis);
    float min = getIndex(minBounds(node->box), node->splitAxis);
    float tMin = (min-origin)/dir;
    float tMax = (max-origin)/dir;
    if(tMin > tMax) {
        float tmp = tMin;
        tMin = tMax;
        tMax = tmp;
    }
    if(tMin > ray.length) return;
    if(tMax < 0) return;
    ray.interSectionTests++;
    if(!findIntersection(ray, node->box)) {return;}
    if(node->childLeft && node->childRight) {
        float leftBoundMin = getIndex(minBounds(node->childLeft->box), node->splitAxis);
        float leftBoundMax = getIndex(maxBounds(node->childLeft->box), node->splitAxis);
        float rightBoundMin = getIndex(minBounds(node->childRight->box), node->splitAxis);
        float rightBoundMax = getIndex(maxBounds(node->childRight->box), node->splitAxis);
        float tMin1 = (leftBoundMin - origin)/dir;
        float tMax1 = (leftBoundMax - origin)/dir;
        
        if(tMin1 > tMax1) {
            float tmp = tMin1;
            tMin1 = tMax1;
            tMax1 = tmp;
        }

        float tMin2 = (rightBoundMin - origin)/dir;
        float tMax2 = (rightBoundMax - origin)/dir;

        if(tMin2 > tMax2) {
            float tmp = tMin2;
            tMin2 = tMax2;
            tMax2 = tmp;
        }
       
        if(tMax1 > 0 && tMin1 < ray.length)findBVHIntesection(ray, node->childLeft, isObject);
        if(tMax2 > 0 && tMin2 < ray.length)findBVHIntesection(ray, node->childRight, isObject);
    } else if(node->childLeft) {
        float leftBoundMin = getIndex(minBounds(node->childLeft->box), node->splitAxis);
        float leftBoundMax = getIndex(maxBounds(node->childLeft->box), node->splitAxis);
        float tMin1 = (leftBoundMin - origin)/dir;
        float tMax1 = (leftBoundMax - origin)/dir;
        if(tMin1 > tMax1) {
            float tmp = tMin1;
            tMin1 = tMax1;
            tMax1 = tmp;
        }
        if(tMax1 > 0 && tMin1 < ray.length) findBVHIntesection(ray, node->childLeft, isObject);
    } else if(node->childRight) {
        float rightBoundMin = getIndex(minBounds(node->childRight->box), node->splitAxis);
        float rightBoundMax = getIndex(maxBounds(node->childRight->box), node->splitAxis);
        float tMin2 = (rightBoundMin - origin)/dir;
        float tMax2 = (rightBoundMax - origin)/dir;

        if(tMin2 > tMax2) {
            float tmp = tMin2;
            tMin2 = tMax2;
            tMax2 = tmp;
        }
        if(tMax2 > 0 && tMin2 < ray.length) findBVHIntesection(ray, node->childRight, isObject);
    }
    
    if(!leaf) return;
    ray.interSectionTests++;
    for (int i = 0; i < node->indices.count; ++i) {
    int idx =  node->indices.data[i];
    if(!isObject)findIntersection(ray, getPrimitive(idx));
    else{findIntersection(ray, getObjectBufferAtIdx(idx));}
    }
}

void constructBVH(BvhNode & node, bool isObject) {
    std::vector<PrimitiveCompare> primitvesAtSplittingAcces(0);
    calculateBoundingBox(node, isObject);
    int split = (int)(rand() % 3);
    node.splitAxis = split;
    if((node.depth == settings.maxDepth) || node.indices.count == 1) {
        if(node.childRight) delete node.childRight;
        if(node.childLeft) delete node.childLeft;
        node.childRight = 0x0;
        node.childLeft = 0x0;
        return;
    }

    void* primitive;
    for (int i = 0; i < node.indices.count; i++) {
        int idx = node.indices.data[i];
        PrimitiveCompare p{idx, 0.0f};
        if(!isObject) primitive = getPrimitive(idx);
        else primitive = getObjectBufferAtIdx(idx);
        if(split == 0) {
            p.val = minBounds(primitive).x;
        } else if(split == 1) {
            p.val = minBounds(primitive).y;
        } else if(split == 2) {
            p.val = minBounds(primitive).z;
        }
        primitvesAtSplittingAcces.push_back(p);
    }
    std::sort(primitvesAtSplittingAcces.begin(), primitvesAtSplittingAcces.end());
    
    
    //median split
    if(!node.childLeft)node.childLeft = new BvhNode; 
    node.childLeft->indices = {};
    node.childLeft->depth = node.depth+1;
    if(!node.childRight)node.childRight = new BvhNode;
    node.childRight->indices = {};
    node.childRight->depth = node.depth+1;
    int size = node.indices.count;
    int halfSize = size / 2;
    for(int i=0; i < halfSize; i++) {
        addToPrimitiveContainer(node.childLeft->indices, primitvesAtSplittingAcces[i].idx);
    }
    for(int i=halfSize; i < size; i++) {
        addToPrimitiveContainer(node.childRight->indices, primitvesAtSplittingAcces[i].idx);
    }
    constructBVH(*node.childLeft, isObject);
    constructBVH(*node.childRight, isObject);
}



BvhNode constructBVH(int startIdx, int endIdx, bool isObject){
    BvhNode root{};
    root.depth = 0;
    std::vector<PrimitiveCompare> primitvesAtSplittingAcces(0);
    int split = (int)(rand() % 3);
    root.splitAxis = split;
    void* primitive;
    //get minBounds of every primtive
    for (int i = startIdx; i < endIdx; i++) {
        PrimitiveCompare p{i, 0.0f};
        if(!isObject) primitive = getPrimitive(i);
        else primitive = getObjectBufferAtIdx(i);
        if(split == 0) {
            p.val = minBounds(primitive).x;
        } else if(split == 1) {
            p.val = minBounds(primitive).y;
        } else if(split == 2) {
            p.val = minBounds(primitive).z;
        }
        primitvesAtSplittingAcces.push_back(p);
        addToPrimitiveContainer(root.indices, i);
    }
    std::sort(primitvesAtSplittingAcces.begin(), primitvesAtSplittingAcces.end());
    //median split
    calculateBoundingBox(root, isObject);
    root.childLeft = new BvhNode;
    root.childLeft->indices = {};
    root.childLeft->depth = root.depth+1;
    root.childRight = new BvhNode;
    root.childRight->indices = {};
    root.childRight->depth = root.depth+1;
    int size = primitvesAtSplittingAcces.size();
    int halfSize = size / 2;
    for(int i=0; i < (halfSize); i++) {
        addToPrimitiveContainer(root.childLeft->indices, primitvesAtSplittingAcces[i].idx);
    }
    for(int i=halfSize; i < size; i++) {
        addToPrimitiveContainer(root.childRight->indices, primitvesAtSplittingAcces[i].idx);
    }
    constructBVH(*root.childLeft, isObject);
    constructBVH(*root.childRight, isObject);
    return root;
}
