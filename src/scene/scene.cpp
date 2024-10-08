#include "scene.h"
#include "accelerationStructures/lightFieldGridSubBeams.h"
#include "common.h"
#include "primitives/object.h"
#include "primitives/primitive.h"
#include "scene/SceneFile.h"
#include "shader/shader.h"
#include "types/camera.h"
#include "accelerationStructures/bvh.h"
#include "accelerationStructures/lightFieldGrid.h"
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
std::vector<Object> lights={};
Material orange{.pbr={}, .weights={},.name=std::string("None")};
int root {};
}

std::vector<u32> & getIndicies() {return indicies;}
std::vector<Triangle> & getTris() {return tris;}
std::vector<Object> & getObjects() {return objects;}
std::vector<Object> & getLights() {return lights;}

Object* getLight(Ray & r) {
    if(lights.size() == 0) return 0x0;
    u32 size = lights.size();
    float f = fastRandom(r.randomState);
    u32 idx = (u32) std::floor((f * (float)size));
    return &lights[idx];
}

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
    else if (getIntersectMode() == TWO_PLANE) {
        intersectGrid(ray);
    }
    else if (getIntersectMode() == SUB_BEAMS) {
        intersectSubBeamGrid(ray);
    }
}


bool scenenInited = false;
void initScene() {
    for(auto m : getSceneFile().models) {
        //loadObject("scene.obj", {0,0,0}, {1,1,1}, addMaterial(orange));
        loadObject(m.file.c_str(), m.pos, m.scale, addMaterial(orange));
    }

    for(auto m : getSceneFile().materials) {
        //loadObject("scene.obj", {0,0,0}, {1,1,1}, addMaterial(orange));
        for(auto & mat : *getMaterials()) {
            if(mat.name != m.name) continue;
            mat.weights = m.mat.weights;
            mat.pbr = m.mat.pbr;
        }
    }

    
    root = constructBVH(0, objects.size());
    for (int i = 0; i < objects.size(); i++) {
        auto & obj = objects[i];
        printf("building bvh for %s\n", obj.name.c_str());
        obj.root = constructBVH(obj.startIdx, obj.endIdx, -1, true);
        printf("finished bvh for %s\n", obj.name.c_str());
        printf("bvh for %d\n", getBVHDepth(obj.root));
    }
    printf("BVH root %d\n", root);
    printf("building grid\n");
    constructGrid();
    printf("2plane %f GB\n", getMemory2Plane() / 1000000000.0f);
    //buildGridsWithSubBeams();
    
    cameraSetForward(getSceneFile().cam.forward);
    getCamera()->origin = getSceneFile().cam.pos;
    getCamera()->dof = getSceneFile().cam.dof;
    getCamera()->focus = getSceneFile().cam.focus;
    resetScene();
    scenenInited = true;
}

void buildAS() {
}

void resetScene() {
    lights.clear();
    for (auto & o : objects) {
        for(auto & mIdx : o.materials) {
            if(getMaterial(mIdx)->pbr.emmision < EPS) continue;
            lights.push_back(o);
            break;
        }
    }

}

void destroyScene() {   
    destroyBVH();
}



