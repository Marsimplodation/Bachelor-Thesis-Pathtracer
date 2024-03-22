#include "scene.h"
#include "../primitives/plane.h"
#include "../primitives/pointLight.h"
#include "../primitives/triangle.h"
#include "../primitives/sphere.h"
#include "../shader/shader.h"
#include "../types/camera.h"
#include "../window/window.h"
#include <cmath>
#include <cstdlib>

namespace {

PrimitivesContainer<Triangle> triangles={};
PrimitivesContainer<Plane> planes={};
PrimitivesContainer<Sphere> spheres={};

SimpleShaderInfo red{.color=Vector3{1.0f, 0.0f, 0.0f}, .shaderFlag=SHADOWSHADER};
SimpleShaderInfo blue{.color={0.0f, 0.0f, 1.0f}, .shaderFlag=SHADOWSHADER};
SimpleShaderInfo white{.color={1.0f, 1.0f, 1.0f}, .shaderFlag=SHADOWSHADER};
SimpleShaderInfo mirror{.color={1.0f, 1.0f, 1.0f}, .shaderFlag=MIRRORSHADER};
SimpleShaderInfo orange{.color={1.0f, 0.6f, 0.0f}, .shaderFlag=SHADOWSHADER};
SimpleShaderInfo glass{.color={1.0f, 1.0f, 1.0f}, .shaderFlag=REFRACTSHADER, .refractiveIdx1 = 1.0f, .refractiveIdx2=1.51f};
PointLight sphereLights[] = {{
.color = {14, 14, 14},
.center = {0,5,0},
.radius = 0.2f,
}};
}


void findIntersection(Ray &ray) {
    float xi = ((float)rand()/RAND_MAX);
    if(xi < KILLCHANCE) {
        ray.terminated = true;
    }

    int num = triangles.count + planes.count + spheres.count; 
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
    }
}


Vector3 getDirectLightSample(Ray & r) {
    int numpLights = sizeof(sphereLights) / sizeof(sphereLights[0]);
    for (int i = 0; i < numpLights; i++) {
        return illuminate(r, sphereLights[i]);
    }
    return {};
}


void initScene() {   
    addToPrimitiveContainer(triangles, createTriangle({0,-3,-4}, {0,-3,0}, {3,-3,-5}, &mirror));
    addToPrimitiveContainer(spheres, createSphere({-3, -2, -1}, 1.5f, &glass));

    addToPrimitiveContainer(planes, createPlane({0,-5,0}, {0,1,0}, &white)); 
    addToPrimitiveContainer(planes, createPlane({0,5,0}, {0,-1,0}, &white)); 
    addToPrimitiveContainer(planes, createPlane({5,0,0}, {-1,0,0}, &red)); 
    addToPrimitiveContainer(planes, createPlane({-5,0,0}, {1,0,0}, &blue)); 
    addToPrimitiveContainer(planes, createPlane({0,0,-10}, {0,0,1}, &white)); 
    addToPrimitiveContainer(planes, createPlane({0,0,10}, {0,0,-1}, &white)); 
    
    
    Vector3 f{0.25f, -0.43f, 1.0f};
    Vector3 u{0.2f, 1.0f, 0.0f};
    registerInfo(getCamera()->origin, "camera origin"); 
    registerInfo(sphereLights[0].center, "light Center");
    registerInfo(sphereLights[0].color, "light Intensity");
    registerInfo(sphereLights[0].radius, "light radius");
    cameraSetForward(f);
    cameraSetUp(u);
}
