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

Material red{.color=Vector3{.65f, 0.05f, 0.05f}, .shaderFlag=SHADOWSHADER, .refractiveIdx2 = 1.0f };
Material green{.color={0.12f, 0.45f, 0.15f}, .shaderFlag=SHADOWSHADER, .refractiveIdx2 = 1.0f };
Material white{.color={0.73f, 0.73f, 0.73f}, .shaderFlag=SHADOWSHADER, .refractiveIdx2 = 1.0f };
Material emit{.color={1.0f, 1.0f, 1.0f}, .shaderFlag=EMITSHADER, .intensity=10.0f};
Material mirror{.color={1.0f, 1.0f, 1.0f}, .shaderFlag=MIRRORSHADER};
Material orange{.color={1.0f, 0.6f, 0.0f}, .shaderFlag=EMITSHADER, .intensity = 1.0f };
Material glass{.color={1.0f, 1.0f, 1.0f}, .shaderFlag=REFRACTSHADER, .refractiveIdx1 = 1.0f, .refractiveIdx2=1.51f};
Material normal{.shaderFlag=static_cast<char>(0xFF)};
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
        ray.interSectionTests++;
        primitive = getPrimitive(idx);
        findIntersection(ray, primitive);
    }
    }
}



bool scenenInited = false;
void initScene() {   
    //addToPrimitiveContainer(triangles, createTriangle({0,-3,-4}, {0,-3,0}, {3,-3,-5}, &mirror));
    addToPrimitiveContainer(spheres, createSphere({300, -150.0f, 0}, 150.0f, addMaterial(glass)));
    //addToPrimitiveContainer(spheres, createSphere({-3, -3.5f, 3}, 1.5f, &mirror));

    addToPrimitiveContainer(cubes, createCube({0,-250,0}, {500,0.1,500}, addMaterial(white))); 
    addToPrimitiveContainer(cubes, createCube({0,250,0}, {500,0.1,500}, addMaterial(white))); 
    addToPrimitiveContainer(cubes, createCube({-250,0,0}, {0.1,500,500}, addMaterial(red))); 
    addToPrimitiveContainer(cubes, createCube({250,0,0}, {0.1,500,500}, addMaterial(green))); 
    //addToPrimitiveContainer(cubes, createCube({0,0,-5}, {10,10,0.1}, addMaterial(white))); 
    addToPrimitiveContainer(cubes, createCube({0,0,250}, {500,500,0.1}, addMaterial(white)));
    addToPrimitiveContainer(cubes, createCube({0,249,2}, {300,1.0f,300}, addMaterial(emit)));
    loadObject("test.obj", {0,-220,50}, {200,200,200}, addMaterial(orange), &objects);
    
    root = constructBVH(0, getNumPrimitives(), false);
    
    Vector3 f{0.05f, -0.15f, 1.0f};
    Vector3 u{0.0f, 1.0f, 0.0f};
    cameraSetForward(f);
    cameraSetUp(u);
    scenenInited = true;
}

void resetScene() {
    if(!scenenInited) return;
    constructBVH(root, false);
    for(int i = 0; i < objects.count; i++) {
        constructBVH(objects.data[i].root, true);
    }
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



