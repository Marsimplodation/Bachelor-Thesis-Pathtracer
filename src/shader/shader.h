#ifndef SHADER_H
#define SHADER_H
#include "types/texture.h"
#include <string>
#define EMITSHADER 0x00
#define SHADOWSHADER 0x01
#define MIRRORSHADER 0x02
#define REFRACTSHADER 0x03
#define EDGESHADER 0x04

#include "../types/vector.h"
#include "../types/ray.h"


struct Material {
    Vector3 color;
    char shaderFlag;
    float refractiveIdx1;
    float refractiveIdx2;
    float intensity;
    Texture texture;
    std::string name;
};

Vector3 shade(Ray &r);
Vector3 emitShader(Ray & r);
Vector3 shadowShader(Ray & r);
Vector3 mirrorShader(Ray & r);
Vector3 refractionShader(Ray & r);
Vector3 edgeShader(Ray & r);
int addMaterial(Material m);
Material * getMaterial(int idx);
std::vector<Material> *getMaterials(); 
#endif // !SHADER_H
