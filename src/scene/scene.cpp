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
#include <string>
#include <vector>


namespace {
std::vector<Triangle> tris={};
std::vector<u32> indicies={};
std::vector<Object> objects={};
Material orange{.pbr={}, .weights={},.name=std::string("None")};
int root {};
}

std::vector<u32> & getIndicies() {return indicies;}
std::vector<Triangle> & getTris() {return tris;}
std::vector<Object> & getObjects() {return objects;}



Vector3 getSceneMaxBounds() {
    Vector3 max{-INFINITY,-INFINITY,-INFINITY};
    for (int i=0; i < objects.size(); ++i) {
        Vector3 pmax = maxBounds(objects[i]); 
        max.x = std::fmaxf(max.x, pmax.x); 
        max.y = std::fmaxf(max.y, pmax.y); 
        max.z = std::fmaxf(max.z, pmax.z); 
    }
    return max;
}

Vector3 getSceneMinBounds() {
    Vector3 min{+INFINITY,+INFINITY,+INFINITY};
    for (int i=0; i < objects.size(); ++i) {
        Vector3 pmin = minBounds(objects[i]); 
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
    for (int i = 0; i < objects.size(); i++) {
        auto & obj = objects[i];
        printf("building bvh for %s\n", obj.name.c_str());
        obj.root = constructBVH(obj.startIdx, obj.endIdx, -1, true);
        printf("finished bvh for %s\n", obj.name.c_str());
    }
    printf("BVH root %d\n", root);
    printf("building grid\n");
    constructGrid();
    
    /*Vector3 f{0.0f, 0.0f, 1.0f};
    Vector3 u{0.0f, 1.0f, 0.0f};
    cameraSetForward(f);
    cameraSetUp(u);*/
    scenenInited = true;
}

void buildAS() {
}

void resetScene() {
}

void destroyScene() {   
    destroyBVH();
}



