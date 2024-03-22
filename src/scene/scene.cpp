#include "scene.h"
#include "../primitives/plane.h"
#include "../primitives/triangle.h"
#include "../primitives/cube.h"
#include "../primitives/sphere.h"
#include "../shader/shader.h"
#include "../types/camera.h"
#include "../window/window.h"
#include <cstdlib>

namespace {

PrimitivesContainer<Triangle> triangles={};
PrimitivesContainer<Plane> planes={};
PrimitivesContainer<Sphere> spheres={};
PrimitivesContainer<Cube> cubes={};

SimpleShaderInfo red{.color=Vector3{1.0f, 0.0f, 0.0f}, .shaderFlag=SHADOWSHADER};
SimpleShaderInfo blue{.color={0.0f, 0.0f, 1.0f}, .shaderFlag=SHADOWSHADER};
SimpleShaderInfo white{.color={1.0f, 1.0f, 1.0f}, .shaderFlag=SHADOWSHADER};
SimpleShaderInfo emit{.color={1.0f, 1.0f, 1.0f}, .shaderFlag=SOLIDSHADER, .intensity=100.0f};
SimpleShaderInfo mirror{.color={1.0f, 1.0f, 1.0f}, .shaderFlag=MIRRORSHADER};
SimpleShaderInfo orange{.color={1.0f, 0.6f, 0.0f}, .shaderFlag=SHADOWSHADER};
SimpleShaderInfo glass{.color={1.0f, 1.0f, 1.0f}, .shaderFlag=REFRACTSHADER, .refractiveIdx1 = 1.0f, .refractiveIdx2=1.51f};
SimpleShaderInfo normal{.shaderFlag=static_cast<char>(0xFF)};
}


void findIntersection(Ray &ray) {
    float xi = ((float)rand()/RAND_MAX);
    if(ray.depth > 2 && xi < KILLCHANCE) {
        ray.terminated = true;
    } else if(ray.depth > 2)
        ray.throughPut *= 1.0f/(1-KILLCHANCE);

    int num = triangles.count + planes.count + spheres.count + cubes.count; 
    for (int i = 0; i < num; i++) {
        if(ray.terminated) return;
        int idx = i;
        if(idx < triangles.count) {
            findIntersection(ray, triangles.data[idx]);
            continue;
        }
        
        idx -= triangles.count;
        if(idx < planes.count) {
            findIntersection(ray, planes.data[idx]);
            continue;
        }
        
        idx -= planes.count;
        if(idx < spheres.count) {
            findIntersection(ray, spheres.data[idx]);
            continue;
        }

        idx -= spheres.count;
        if(idx < cubes.count) {
            findIntersection(ray, cubes.data[idx]);
            continue;
        }
    }
}



void initScene() {   
    addToPrimitiveContainer(triangles, createTriangle({0,-3,-4}, {0,-3,0}, {3,-3,-5}, &mirror));
    addToPrimitiveContainer(spheres, createSphere({-3, -2, -1}, 1.5f, &glass));
    addToPrimitiveContainer(spheres, createSphere({2, 0, 3}, 1.2f, &orange));

    addToPrimitiveContainer(planes, createPlane({0,-5,0}, {0,1,0}, &white)); 
    addToPrimitiveContainer(planes, createPlane({0,5,0}, {0,-1,0}, &white)); 
    addToPrimitiveContainer(planes, createPlane({5,0,0}, {-1,0,0}, &red)); 
    addToPrimitiveContainer(planes, createPlane({-5,0,0}, {1,0,0}, &blue)); 
    addToPrimitiveContainer(planes, createPlane({0,0,-15}, {0,0,1}, &white)); 
    addToPrimitiveContainer(planes, createPlane({0,0,10}, {0,0,-1}, &white));
    addToPrimitiveContainer(cubes, createCube({0,5,1}, {4,1,4}, &emit));
    
    
    Vector3 f{0.05f, -0.15f, 1.0f};
    Vector3 u{0.0f, 1.0f, 0.0f};
    registerInfo(getCamera()->origin, "camera origin"); 
    registerInfo(cubes.data[0].center, "light Center");
    registerInfo(cubes.data[0].size, "light Size");
    registerInfo(emit.color, "light Color");
    registerInfo(emit.intensity, "light intensity");
    cameraSetForward(f);
    cameraSetUp(u);
}
