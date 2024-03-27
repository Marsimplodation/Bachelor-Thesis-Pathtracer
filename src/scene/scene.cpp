#include "scene.h"
#include "common.h"
#include "primitives/primitive.h"
#include "shader/shader.h"
#include "types/camera.h"
#include "types/bvh.h"
#include "types/vector.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace {
PrimitivesContainer<Triangle> triangles={};
PrimitivesContainer<Plane> planes={};
PrimitivesContainer<Sphere> spheres={};
PrimitivesContainer<Cube> cubes={};
PrimitivesContainer<Triangle> objectBuffer={};
PrimitivesContainer<Object> objects={};

SimpleShaderInfo red{.color=Vector3{.65f, 0.05f, 0.05f}, .shaderFlag=SHADOWSHADER};
SimpleShaderInfo green{.color={0.12f, 0.45f, 0.15f}, .shaderFlag=SHADOWSHADER};
SimpleShaderInfo white{.color={0.73f, 0.73f, 0.73f}, .shaderFlag=SHADOWSHADER};
SimpleShaderInfo emit{.color={1.0f, 1.0f, 1.0f}, .shaderFlag=EMITSHADER, .intensity=10.0f};
SimpleShaderInfo mirror{.color={1.0f, 1.0f, 1.0f}, .shaderFlag=MIRRORSHADER};
SimpleShaderInfo orange{.color={1.0f, 0.6f, 0.0f}, .shaderFlag=SHADOWSHADER};
SimpleShaderInfo glass{.color={1.0f, 1.0f, 1.0f}, .shaderFlag=REFRACTSHADER, .refractiveIdx1 = 1.0f, .refractiveIdx2=1.51f};
SimpleShaderInfo normal{.shaderFlag=static_cast<char>(0xFF)};
BvhNode root {};
}

//----- Container access----//
Triangle *getObjectBufferAtIdx(int idx) {
    if (idx >= objectBuffer.count) {
        printf("does not exist");
        return 0x0;
    }
    return &objectBuffer.data[idx];
}

PrimitivesContainer<Triangle> *getObjectBuffer() {
    return &objectBuffer;
}

int getNumPrimitives() {
    int num = triangles.count + planes.count + spheres.count + cubes.count + objects.count;
    return num;
}

void *getPrimitive(int idx) {
    if(idx < triangles.count) {
        return &triangles.data[idx];
    }
    
    idx -= triangles.count;
    if(idx < planes.count) {
        return &planes.data[idx];
    }
    
    idx -= planes.count;
    if(idx < spheres.count) {
        return &spheres.data[idx];
    }

    idx -= spheres.count;
    if(idx < cubes.count) {
        return &cubes.data[idx];
    }

    idx -= cubes.count;
    if(idx < objects.count) {
        return &objects.data[idx];
    }
    return 0x0;
}


void findIntersection(Ray &ray) {
    //    float xi = ((float)rand()/RAND_MAX);
    if(ray.terminated) return;
    float xi = (fastRandom(ray.randomState));
    if(ray.depth > 2 && xi < KILLCHANCE) {
        ray.terminated = true;
    } else if(ray.depth > 2)
        ray.throughPut *= 1.0f/(1-KILLCHANCE);
    
    //bvh
    //
    if(getIntersectMode()==BVH) findBVHIntesection(ray, &root);
    else {
    //normal
    int num = triangles.count + planes.count + spheres.count + cubes.count + objects.count; 
    void * primitive;
    for (int i = 0; i < num; i++) {
        if(ray.terminated) return;
        int idx = i;
        primitive = getPrimitive(idx);
        findIntersection(ray, primitive);
    }
    }
}



bool scenenInited = false;
void initScene() {   
    //addToPrimitiveContainer(triangles, createTriangle({0,-3,-4}, {0,-3,0}, {3,-3,-5}, &mirror));
    addToPrimitiveContainer(spheres, createSphere({3, -1.5f, 0}, 1.5f, &glass));
    //addToPrimitiveContainer(spheres, createSphere({-3, -3.5f, 3}, 1.5f, &mirror));

    addToPrimitiveContainer(cubes, createCube({0,-5,0}, {10,0.1,10}, &white)); 
    addToPrimitiveContainer(cubes, createCube({0,5,0}, {10,0.1,10}, &white)); 
    addToPrimitiveContainer(cubes, createCube({-5,0,0}, {0.1,10,10}, &red)); 
    addToPrimitiveContainer(cubes, createCube({5,0,0}, {0.1,10,10}, &green)); 
    //addToPrimitiveContainer(cubes, createCube({0,0,-5}, {10,10,0.1}, &white)); 
    addToPrimitiveContainer(cubes, createCube({0,0,5}, {10,10,0.1}, &white));
    addToPrimitiveContainer(cubes, createCube({0,5-0.1f,2}, {4,0.1f,4}, &emit));
    addToPrimitiveContainer(objects, loadObject("test.obj", {-1,0,3}, {1,1,1}, &orange));
    
    root = constructBVH(0, getNumPrimitives(), false);
    
    Vector3 f{0.05f, -0.15f, 1.0f};
    Vector3 u{0.0f, 1.0f, 0.0f};
    cameraSetForward(f);
    cameraSetUp(u);
}

void resetScene() {
    if(!scenenInited) return;
    //destroyBVH(root.childLeft);
    //destroyBVH(root.childRight);
    //root = constructBVH(0, getNumPrimitives(), false);
}

void destroyScene() {   
    destroyBVH(root.childLeft);
    destroyBVH(root.childRight);
    destroyContainer(objects); 
    destroyContainer(triangles); 
    destroyContainer(objectBuffer); 
    destroyContainer(spheres); 
    destroyContainer(cubes); 
    destroyContainer(planes);

}



