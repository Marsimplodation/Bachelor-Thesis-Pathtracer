#include "bvh.h"
#include "common.h"
#include "scene/scene.h"
#include "shader/shader.h"
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
    Vector3 tmin, tmax{};
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
    if(!findIntersection(ray, node->box)) {return;}
    
    ray.interSectionTests++;
    bool leaf = !node->childLeft && !node->childRight; 
    findBVHIntesection(ray, node->childLeft, isObject);
    findBVHIntesection(ray, node->childRight, isObject);
    
    if(!leaf) return;
    int idx =  node->indices.data[0];
    if(!isObject)findIntersection(ray, getPrimitive(idx));
    else{findIntersection(ray, getObjectBufferAtIdx(idx));}
}

void constructBVH(BvhNode & node, bool isObject) {
    std::vector<PrimitiveCompare> primitvesAtSplittingAcces(0);
    calculateBoundingBox(node, isObject);
    int split = (int)(rand() % 3);
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
