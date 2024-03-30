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
SimpleShaderInfo red{.color{1,0,0}, .shaderFlag=SHADOWSHADER, .intensity = 1.0f};
}

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
    
    node.box = {.center = center, .size=size*2};
}

void findBVHIntesection(Ray & ray, BvhNode * node, bool isObject) {
    if(!node) return;
    if(ray.terminated) return;
    bool leaf = !node->childLeft && !node->childRight; 
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

    if(!findIntersection(ray, node->box)) {return;}
    //test which to test first
    if(node->childLeft && node->childRight) {
        float leftBound = getIndex(minBounds(node->childLeft->box), node->splitAxis);
        float rightBound = getIndex(minBounds(node->childRight->box), node->splitAxis);
        BvhNode* firstChild = (origin < rightBound) ? node->childLeft : node->childRight;
        BvhNode* secondChild = (origin < rightBound) ? node->childRight : node->childLeft;
        findBVHIntesection(ray, firstChild, isObject);
        findBVHIntesection(ray, secondChild, isObject); 
    } else {
        if(node->childLeft) findBVHIntesection(ray, node->childLeft, isObject);
        if(node->childRight) findBVHIntesection(ray, node->childRight, isObject);
    }
    
    if(!leaf) return;
    ray.interSectionTests++;
    int idx =  node->indices.data[0];
    if(!isObject)findIntersection(ray, getPrimitive(idx));
    else{findIntersection(ray, getObjectBufferAtIdx(idx));}
}

void constructBVH(BvhNode & node, bool isObject) {
    std::vector<PrimitiveCompare> primitvesAtSplittingAcces(0);
    calculateBoundingBox(node, isObject);
    int split = (int)(rand() % 3);
    node.splitAxis = split;
    if(node.indices.count == 1) {
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
    if(!node.childRight)node.childRight = new BvhNode;
    node.childRight->indices = {};
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
    root.childRight = new BvhNode;
    root.childRight->indices = {};
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
