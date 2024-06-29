#include "scene.h"
#include "common.h"
#include "primitives/primitive.h"
#include "shader/shader.h"
#include "types/camera.h"
#include "types/bvh.h"
#include "types/lightFieldGrid.h"
#include "types/vector.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace {
std::vector<Triangle> objectBuffer={};
std::vector<Object> objects={};

Material red{.color=Vector3{.65f, 0.05f, 0.05f}, .shaderFlag=SHADOWSHADER, .refractiveIdx2 = 1.0f };
Material green{.color={0.12f, 0.45f, 0.15f}, .shaderFlag=SHADOWSHADER, .refractiveIdx2 = 1.0f };
Material white{.color={0.73f, 0.73f, 0.73f}, .shaderFlag=SHADOWSHADER, .refractiveIdx2 = 1.0f };
Material emit{.color={1.0f, 1.0f, 1.0f}, .shaderFlag=EMITSHADER, .intensity=10.0f};
Material mirror{.color={1.0f, 1.0f, 1.0f}, .shaderFlag=MIRRORSHADER};
Material orange{.color={1.0f, 0.6f, 0.0f}, .shaderFlag=EMITSHADER, .intensity = 1.0f };
Material glass{.color={1.0f, 1.0f, 1.0f}, .shaderFlag=REFRACTSHADER, .refractiveIdx1 = 1.0f, .refractiveIdx2=1.51f};
Material normal{.shaderFlag=static_cast<char>(0xFF)};
int root {};
}

//----- Container access----//
Triangle *getObjectBufferAtIdx(int idx) {
    if (idx >= objectBuffer.size()) {
        printf("does not exist");
        return 0x0;
    }
    return &objectBuffer[idx];
}

std::vector<Triangle> *getObjectBuffer() {
    return &objectBuffer;
}

int getNumPrimitives() {
    int num = objects.size();
    return num;
}

Vector3 getSceneMaxBounds() {
    Vector3 max{-INFINITY,-INFINITY,-INFINITY};
    for (int i=0; i < objectBuffer.size(); ++i) {
        Vector3 pmax = minBounds(objectBuffer.at(i)); 
        max.x = std::fmaxf(max.x, pmax.x); 
        max.y = std::fmaxf(max.y, pmax.y); 
        max.z = std::fmaxf(max.z, pmax.z); 
    }
    return max;
}

Vector3 getSceneYMaxPoint() {
    Vector3 max{-INFINITY,-INFINITY,-INFINITY};
    for (int i=0; i < objectBuffer.size(); ++i) {
        Vector3 pmax = minBounds(objectBuffer.at(i)); 
        if(pmax.y >= max.y) max = pmax;
    }
    return max;
}



Vector3 getSceneYMinPoint() {
    Vector3 min{+INFINITY,+INFINITY,+INFINITY};
    for (int i=0; i < objectBuffer.size(); ++i) {
        Vector3 pmin = minBounds(objectBuffer.at(i));
        if(pmin.y <= min.y) min = pmin;
    }
    return min;

}

Vector3 getSceneMinBounds() {
    Vector3 min{+INFINITY,+INFINITY,+INFINITY};
    for (int i=0; i < objectBuffer.size(); ++i) {
        Vector3 pmin = minBounds(objectBuffer.at(i)); 
        min.x = std::fminf(min.x, pmin.x); 
        min.y = std::fminf(min.y, pmin.y); 
        min.z = std::fminf(min.z, pmin.z); 
    }
    return min;
}

//idx is 1 in the first bit if object idx
void *getPrimitive(int idx) {
    if(idx < 0 && -idx <= objectBuffer.size()) {
        return &objectBuffer[-idx - 1];
    } else if(idx < 0) {
        return 0x0;
    }

    if(idx < objects.size()) {
        return &objects[idx];
    }

    idx -= objects.size();
    

    return 0x0;
}

void removePrimitive(int idx) {
    if(idx < objects.size()) {
        objects[idx].active = !objects[idx].active;
    }
}

void findIntersection(Ray &ray) {
    //    float xi = ((float)rand()/RAND_MAX);
    if(ray.terminated) return;
    float xi = (fastRandom(ray.randomState));

    if(ray.depth > 2 && xi * max(ray.throughPut) < KILLCHANCE) {
        ray.terminated = true;
    } else if(ray.depth > 2)
        ray.throughPut *= 1.0f/((1-KILLCHANCE)*max(ray.throughPut));
    
    //bvh
    //
    if(getIntersectMode()==BVH) findBVHIntesection(ray, root);
    else if (getIntersectMode() == ALL) {
    //normal
    int num = getNumPrimitives(); 
    void * primitive;
    for (int i = 0; i < num; i++) {
        if(ray.terminated) return;
        int idx = i;
        ray.interSectionTests++;
        findIntersection(ray, idx);
    }
    }
    else if (getIntersectMode() == GRID) {
        intersectGrid(ray);
    }
    else if (getIntersectMode() == HYBRID) {
        if(ray.depth == 0) intersectGrid(ray);
        else findBVHIntesection(ray, root);
    }
}



bool scenenInited = false;
void initScene() {   
    //addToPrimitiveContainer(triangles, createTriangle({0,-3,-4}, {0,-3,0}, {3,-3,-5}, &mirror));
    /*addToPrimitiveContainer(spheres, createSphere({300, -150.0f, 0}, 150.0f, addMaterial(glass)));
    //addToPrimitiveContainer(spheres, createSphere({-3, -3.5f, 3}, 1.5f, &mirror));

    addToPrimitiveContainer(cubes, createCube({0,-250,0}, {500,0.1,500}, addMaterial(white))); 
    addToPrimitiveContainer(cubes, createCube({0,250,0}, {500,0.1,500}, addMaterial(white))); 
    addToPrimitiveContainer(cubes, createCube({-250,0,0}, {0.1,500,500}, addMaterial(red))); 
    addToPrimitiveContainer(cubes, createCube({250,0,0}, {0.1,500,500}, addMaterial(green))); 
    //addToPrimitiveContainer(cubes, createCube({0,0,-5}, {10,10,0.1}, addMaterial(white))); 
    addToPrimitiveContainer(cubes, createCube({0,0,250}, {500,500,0.1}, addMaterial(white)));
    addToPrimitiveContainer(cubes, createCube({0,249,2}, {300,1.0f,300}, addMaterial(emit)));*/
    loadObject("cornel.obj", {0,-250,50}, {300,300,300}, addMaterial(orange), &objects);
    
    root = constructBVH(0, getNumPrimitives());
    constructGrid();
    printf("BVH root %d", root);
    
    /*Vector3 f{0.0f, 0.0f, 1.0f};
    Vector3 u{0.0f, 1.0f, 0.0f};
    cameraSetForward(f);
    cameraSetUp(u);*/
    scenenInited = true;
}

void buildAS() {
    auto const tmp = getIntersectMode();
    setIntersectMode(ALL);
    destroyBVH();
    for(int i = 0; i < objects.size(); i++) {
        objects[i].root = constructBVH(objects[i].startIdx, objects[i].endIdx);
    }
    root = constructBVH(0, getNumPrimitives());
    constructGrid();
    setIntersectMode(tmp);
}

void resetScene() {
    if(!scenenInited) return;
    destroyBVH();
    for(int i = 0; i < objects.size(); i++) {
        constructBVH(objects[i].startIdx, objects[i].endIdx);
    }
    root = constructBVH(0, getNumPrimitives());
    //destroyBVH(root.childLeft);
    //destroyBVH(root.childRight);
    //root = constructBVH(0, getNumPrimitives(), false);
}

void destroyScene() {   
    destroyBVH();
}



