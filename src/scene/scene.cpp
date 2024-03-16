#include "scene.h"
#include "../primitives/plane.h"
#include "../primitives/triangle.h"
#include "../primitives/sphere.h"
#include "../shader/shader.h"
#include "../types/camera.h"
#include "../window/window.h"
#include <cstdlib>
namespace {
Triangle triangles[1];
Plane planes[6];
Sphere spheres[1];
SimpleShaderInfo red{.color=Vector3{1.0f, 0.0f, 0.0f}};
    SimpleShaderInfo blue{.color={0.0f, 0.0f, 1.0f}};
    SimpleShaderInfo white{.color={1.0f, 1.0f, 1.0f}};
    SimpleShaderInfo orange{.color={1.0f, 0.6f, 0.0f}};
}


void findIntersection(Ray &ray) {
    int numTris = sizeof(triangles) / sizeof(triangles[0]);
    float xi = ((float)rand()/RAND_MAX);
    xi -= (1.0f-ray.throughPut);
    if(ray.depth > 3 && xi < KILLCHANCE) {
        ray.terminated = true;
    }
    for (int i = 0; i < numTris; i++) {
        triangleIntersect(ray, triangles[i]);
    }
    int numPlanes = sizeof(planes) / sizeof(planes[0]);
    for (int i = 0; i < numPlanes; i++) {
        planeIntersect(ray, planes[i]);
    }

    int numSpheres = sizeof(spheres) / sizeof(spheres[0]);
    for (int i = 0; i < numSpheres; i++) {
        sphereIntersect(ray, spheres[i]);
    }
}

void initScene() {
   triangles[0] = {
        .vertices =
            {
                {0, -3.0, -4.0},
                {0, -3.0f, 0},
                {3.0, -3.0f, -5.0},

            },
        .normal = normalized({0, 0, -1}),
    };
    triangles[0].normal = normalized(
        crossProduct(triangles[0].vertices[1] - triangles[0].vertices[0],
                     triangles[0].vertices[2] - triangles[0].vertices[0]));
    triangles[0].shaderFlag = MIRRORSHADER;
    triangles[0].shaderInfo = &white;

    spheres[0].center = {-3, -2, -1};
    spheres[0].radius = 1.5f;
    spheres[0].shaderFlag = SHADOWSHADER;
    spheres[0].shaderInfo = (void*)&orange;


    planes[0].normal = {0.0, 1.0, 0.0};
    planes[0].center = {0.0, -5.0, 0.0};
    planes[0].shaderFlag = SHADOWSHADER;
    planes[0].shaderInfo = (void*)&white;

    planes[1].normal = {0.0, -1.0, 0.0};
    planes[1].center = {0.0, 5.0, 0.0};
    planes[1].shaderFlag = SHADOWSHADER;
    planes[1].shaderInfo = (void*)&white;

    planes[2].normal = {-1.0, 0.0, 0.0};
    planes[2].center = {5.0, 0.0, 0.0};
    planes[2].shaderFlag = SHADOWSHADER;
    planes[2].shaderInfo = &red;

    planes[3].normal = {1.0, 0.0, 0.0};
    planes[3].center = {-5.0, 0.0, 0.0};
    planes[3].shaderFlag = SHADOWSHADER;
    planes[3].shaderInfo = &blue;

    planes[4].normal = {0.0, 0.0, -1.0};
    planes[4].center = {0.0, 0.0, 10.0};
    planes[4].shaderFlag = SHADOWSHADER;
    planes[4].shaderInfo = &white;

    planes[5].normal = {0.0, 0.0, 1.0};
    planes[5].center = {0.0, 0.0, -10.0};
    planes[5].shaderFlag = SHADOWSHADER;
    planes[5].shaderInfo = &white;
    
    
    Vector3 f{0.25f, -0.43f, 1.0f};
    Vector3 u{0.2f, 1.0f, 0.0f};
    registerInfo(getCamera()->origin, "camera origin"); 
    registerInfo(*getLight(), "light Intensity");
    cameraSetForward(f);
    cameraSetUp(u);
}
