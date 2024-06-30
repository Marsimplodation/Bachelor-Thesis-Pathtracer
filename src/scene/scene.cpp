#include "scene.h"
#include "common.h"
#include "primitives/object.h"
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
std::vector<Triangle> tris={};
std::vector<u32> indicies={};
std::vector<Object> objects={};
Material orange{.color={1.0f, 0.6f, 0.0f}, .shaderFlag=EMITSHADER, .intensity = 1.0f };
int root {};
}

std::vector<u32> & getIndicies() {return indicies;}
std::vector<Triangle> & getTris() {return tris;}
std::vector<Object> & getObjects() {return objects;}



Vector3 getSceneMaxBounds() {
    Vector3 max{-INFINITY,-INFINITY,-INFINITY};
    for (int i=0; i < objects.size(); ++i) {
        Vector3 pmax = minBounds(objects.at(i)); 
        max.x = std::fmaxf(max.x, pmax.x); 
        max.y = std::fmaxf(max.y, pmax.y); 
        max.z = std::fmaxf(max.z, pmax.z); 
    }
    return max;
}

Vector3 getSceneMinBounds() {
    Vector3 min{+INFINITY,+INFINITY,+INFINITY};
    for (int i=0; i < objects.size(); ++i) {
        Vector3 pmin = minBounds(objects.at(i)); 
        min.x = std::fminf(min.x, pmin.x); 
        min.y = std::fminf(min.y, pmin.y); 
        min.z = std::fminf(min.z, pmin.z); 
    }
    return min;
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
    void * primitive;
    for (int i = 0; i < objects.size(); i++) {
        if(ray.terminated) return;
        int idx = i;
        ray.interSectionTests++;
        objectIntersection(ray, objects[idx]);
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
    loadObject("cornel.obj", {0,-250,50}, {300,300,300}, addMaterial(orange));
    
    root = constructBVH(0, objects.size());
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
        objects[i].root = constructBVH(objects[i].startIdx, objects[i].endIdx, -1, true);
    }
    root = constructBVH(0, objects.size());
    constructGrid();
    setIntersectMode(tmp);
}

void resetScene() {
    if(!scenenInited) return;
    destroyBVH();
    for(int i = 0; i < objects.size(); i++) {
        constructBVH(objects[i].startIdx, objects[i].endIdx, -1, true);
    }
    root = constructBVH(0, objects.size());
    //destroyBVH(root.childLeft);
    //destroyBVH(root.childRight);
    //root = constructBVH(0, getNumPrimitives(), false);
}

void destroyScene() {   
    destroyBVH();
}



