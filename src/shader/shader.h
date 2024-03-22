#ifndef SHADER_H
#define SHADER_H
#define SOLIDSHADER 0x00
#define SHADOWSHADER 0x01
#define MIRRORSHADER 0x02
#define REFRACTSHADER 0x03

#include "../types/vector.h"
#include "../types/ray.h"


struct SimpleShaderInfo {
    Vector3 color;
    char shaderFlag;
    float refractiveIdx1;
    float refractiveIdx2;
    float intensity;
};

Vector3 shade(Ray & r);
Vector3 solidShader(Ray & r);
Vector3 shadowShader(Ray & r);
Vector3 mirrorShader(Ray & r);
Vector3 refractionShader(Ray & r);

#endif // !SHADER_H
