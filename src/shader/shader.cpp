#include "shader.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "../scene/scene.h"
namespace {
}

float calculateFresnelTerm(float dot, float n1, float n2) {
    float r0 = ((n1 -n2)/(n1+n2));
    r0*=r0;
    return r0 + (1-r0) * pow(1-dot, 5);
}

void realistic(Ray & r, float scale) {
    if(r.depth > 0 && r.length != MAXFLOAT) {
        float l = r.length*scale;
        float f  = 1.0/(l*l);
        if(f > 1.0f) f = 1.0f;
        r.throughPut *= f;
        r.throughPut *= fabsf(dotProduct(r.normal, r.direction));
    }

}

Vector3 shade(Ray &r, float scale) {
    Vector3 black{0.0f, 0.0f, 0.0f};
    if (!r.hit) return black;
    realistic(r, scale); //always take distance into account
    switch (((SimpleShaderInfo*)r.shaderInfo)->shaderFlag) {
        case SOLIDSHADER:
            return solidShader(r);
        case MIRRORSHADER: 
            return mirrorShader(r);
        case SHADOWSHADER:
            return shadowShader(r);
        case REFRACTSHADER:
            return refractionShader(r);
        default:
            return r.normal;
    }
}

Vector3 mirrorShader(Ray & r) {
    r.origin = r.origin + r.direction * (r.length);
    r.direction = r.direction - 2.0f* dotProduct(r.direction, r.normal)*r.normal;
    r.origin += r.direction * 0.001f;
    r.length = MAXFLOAT;
    r.throughPut *= 1.0f;
    r.hit=false;
    return {};
}

Vector3 solidShader(Ray &r) {
    SimpleShaderInfo * info = (SimpleShaderInfo*) r.shaderInfo;
    //r.terminated = true;
    return clampToOne((info)->color * r.colorMask * info->intensity * r.throughPut);
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
        float tmp = n2;
        //n2 = n1;
        //n1 = tmp;
    }
    
    Vector3 refractDirection;
    float discriminator = 1.0f - (eta * eta) * (1.0f - (cos * cos));

    //internal relection
    float xi = (float)rand()/RAND_MAX;
    float reflectance = calculateFresnelTerm(-cos, n1, n2);
    if(reflectance > 1) reflectance = 1;
    if(reflectance < 0) reflectance = 0;
    if(discriminator < eps || xi < reflectance){
        refractDirection = normalized(r.direction - 2.0f* dotProduct(r.direction, r.normal)*r.normal);
        //if(discriminator > eps)
            //r.throughPut *= reflectance * 1.0f / (reflectance);  // r * 1/r = 1
    }
    else{
        refractDirection = normalized(eta * (r.direction - cos * normal) - normal * sqrtf(discriminator+eps));
        //r.throughPut *= 1 - reflectance;    
        //r.throughPut *= 1.0f / (1-reflectance);    
    }
    
    r.origin = r.origin +r.direction * (r.length) + refractDirection*eps;
    r.direction = refractDirection;
    r.length = MAXFLOAT;
    r.hit=false;
    //r.colorMask = r.colorMask * info->color;

    return {};
}

Vector3 shadowShader(Ray &r) {
    SimpleShaderInfo * in = (SimpleShaderInfo*) r.shaderInfo;
    auto fgColor = in->color;
    fgColor = fgColor * r.colorMask;
    r.colorMask = r.colorMask * in->color;

    //reset for next bounce
    r.origin = r.origin + r.direction * r.length;
    r.direction = randomV3UnitHemisphere(r.normal);
    r.origin += r.direction*0.01;
    r.throughPut *= fabsf(dotProduct(r.normal, r.direction));
    r.length = MAXFLOAT;
    r.hit=false;
    
    return {};// fgColor * lightColor  * r.throughPut;
}
