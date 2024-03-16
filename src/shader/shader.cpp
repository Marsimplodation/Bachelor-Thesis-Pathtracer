#include "shader.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "../scene/scene.h"
namespace {
float light = 0.0f;
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
        case REFRACTSHADER:
            return refractionShader(r);
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
    r.hit=false;
    return {};
}

float *getLight() {
    return &light;
}

Vector3 solidShader(Ray &r, void *info) {
    return ((SimpleShaderInfo*)info)->color * r.colorMask;

}

Vector3 refractionShader(Ray &r) {
    SimpleShaderInfo * info = (SimpleShaderInfo*) r.shaderInfo;
        
    float eps = 0.001f;
    Vector3 normal = r.normal;
    float n1 = info->refractiveIdx1;
    float n2 = info->refractiveIdx2;
    float eta = n1 / n2;
    float cos = dotProduct(r.direction, r.normal);

    if(cos >= eps) { //in object
        cos *= -1;
        eta = 1.0f / eta;
        normal = normal * -1;
    }
    
    Vector3 refractDirection;
    float discriminator = 1.0f - (eta * eta) * (1.0f - (cos * cos));

    //internal relection
    if(discriminator < eps)  refractDirection = - 2.0f* dotProduct(r.direction, r.normal)*r.normal;
    else refractDirection = normalized(eta* (r.direction - cos * normal) - normal * sqrtf(discriminator+eps));
    
    r.origin = r.origin +r.direction * (r.length) + refractDirection*eps;
    r.direction = refractDirection;
    r.terminated = false;
    r.length = MAXFLOAT;
    r.hit=false;

    return {};
}

Vector3 shadowShader(Ray &r, void *info) {
    SimpleShaderInfo * in = (SimpleShaderInfo*) r.shaderInfo;
    auto fgColor = in->color;
    fgColor = fgColor * r.colorMask;
    r.colorMask = r.colorMask * in->color;
    
    //light
    Vector3 lightColor = getDirectLightSample(r);

    //reset for next bounce
    r.origin = r.origin + r.direction * r.length;
    r.direction = randomV3UnitHemisphere(r.normal);
    //attenuation for secondary rays
    if(r.depth > 0 && r.length != MAXFLOAT) {
        //fix later
        float f  = 1.0f;//(r.length*r.length);
        if(f > 1.0f) f = 1.0f;
        r.throughPut *= f;
        r.throughPut *= fabs(dotProduct(r.normal, r.direction));
    }
    r.length = MAXFLOAT;
    
    return fgColor * lightColor  * r.throughPut;
}
