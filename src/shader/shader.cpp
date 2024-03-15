#include "shader.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "../scene/scene.h"
namespace {
float light = 1.0f;
}

Vector3 shade(Ray &r) {
    Vector3 black{0.0f, 0.0f, 0.0f};
    if (!r.hit) return black;
    switch (r.shaderFlag) {
        case SOLIDSHADER:
            return solidShader(r, r.shaderInfo);
        case MIRRORSHADER: 
            return mirrorShader(r);
        case SHADOWSHADER:
            return shadowShader(r, r.shaderInfo);
        default:
            return r.direction;
    }
}

Vector3 mirrorShader(Ray & r) {
    r.origin = r.origin + r.direction * r.length;
    r.direction = r.direction - 2.0f* dotProduct(r.direction, r.normal)*r.normal;
    r.terminated = false;
    r.length = MAXFLOAT;
    r.throughPut = 1.0f;
    r.depth--;
    r.hit=false;
    return {};
}

float *getLight() {
    return &light;
}

Vector3 solidShader(Ray &r, void *info) {
    return ((SimpleShaderInfo*)info)->color;
}

Vector3 shadowShader(Ray &r, void *info) {
    SimpleShaderInfo * in = (SimpleShaderInfo*) r.shaderInfo;
    auto fgColor = in->color;
    fgColor = fgColor * r.colorMask;
    r.colorMask = r.colorMask * in->color;
    
    //light
    float radius = 0.2f;
    float xi1 = (((float)rand()/RAND_MAX)*2-1.0f) *radius;
    float xi2 = (((float)rand()/RAND_MAX)*2 -1.0f)*radius;
    float xi3 = (((float)rand()/RAND_MAX)*2 -1.0f) *radius;
    Vector3 lightColor{1,1,1};
    Vector3 lightPos{0.0f + xi1, 0.0f+xi2, 0.0f+xi3};
   
    //direct illumination
    Ray shadowRay = r;
    shadowRay.hit = false;
    shadowRay.origin = r.origin +r.direction * (r.length+0.01f);
    shadowRay.direction = normalized(lightPos - shadowRay.origin);
    shadowRay.length = length(lightPos-shadowRay.origin);
    findIntersection(shadowRay);
    float i = 1.0f/(shadowRay.length * shadowRay.length);
    if (i >= 1.0f) i=1.0f;
    i*= fabs(dotProduct(r.normal, shadowRay.direction));
    i*=light; // intensity
    i*=1.0f/(1.0f-KILLCHANCE); // russian roullete

    //reset for next bounce
    r.origin = r.origin + r.direction * r.length;
    r.direction = randomV3UnitHemisphere(r.normal);
    //attenuation for secondary rays
    if(r.depth > 0 && r.length != MAXFLOAT) {
        float f  = 1.0f/(r.length*r.length);
        if(f > 1.0f) f = 1.0f;
        r.throughPut *= f;
        r.throughPut *= fabs(dotProduct(r.normal, r.direction));
    }
    r.length = MAXFLOAT;
    
    if(shadowRay.hit) i=0;
    return fgColor * lightColor * i * r.throughPut;
}
