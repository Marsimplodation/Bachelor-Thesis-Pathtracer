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

struct ShaderMix {
    float lambert = 1.0f;
    float reflection = 0.0f;
    float refraction = 0.0f;
};

struct SkyBox {
    bool isActive = false;
    float emmision = 1.0f;
    Texture texture;
};

struct VolumetricFog {
    Vector3 absorption = {1,1,1};
    float density = 0.3f;
    float coef = 5.0f;
    bool isActive = false;
};
struct pbrProbs {
    Vector3 albedo;
    float refractiveIdx1 = 1.0f;
    float refractiveIdx2 = 1.5f;
    float emmision = 0.0f;
    float roughness = 0.5f;

    Texture texture;
    Texture normal;
}; 

bool &getPrimaryOnly();
struct Material {
    pbrProbs pbr;
    ShaderMix weights;
    std::string name;
};

bool &getNEE();
Vector3 shade(Ray &r);
int addMaterial(Material m);
Material * getMaterial(int idx);
std::vector<Material> *getMaterials(); 
VolumetricFog &getVolumetricFog();
SkyBox & getSkyBox();
#endif // !SHADER_H
