#include "shader.h"
#include "common.h"
#include "primitives/object.h"
#include "primitives/triangle.h"
#include "scene/scene.h"
#include "types/aabb.h"
#include "types/ray.h"
#include "types/texture.h"
#include "types/vector.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
namespace {
#define GAMMA 2.2f
std::vector<Material> materials;
bool nee = true;
bool primaryOnly = false;
} // namespace

Material *getMaterial(int idx) { return &materials[idx]; }
bool &getPrimaryOnly() {
    return primaryOnly;
}


std::vector<Material> *getMaterials() { return &materials; }

bool &getNEE() {return nee;}

int addMaterial(Material m) {
    int idx = materials.size();
    if (m.name.empty())
        m.name = "Material";
    materials.push_back(m);
    return idx;
}

void gammaCorrect(Vector3 &c) {
    // convert to HDR
    c[0] = std::powf(c[0], GAMMA);
    c[1] = std::powf(c[1], GAMMA);
    c[2] = std::powf(c[2], GAMMA);
}

float calculateFresnelTerm(float dot, float n1, float n2) {
    float r0 = ((n1 - n2) / (n1 + n2));
    r0 *= r0;
    return r0 + (1 - r0) * pow(1 - dot, 5);
}

Vector3 getColorOfMaterial(Ray & r, Material & info) {
    Vector3 color;
    if (info.pbr.texture.data.empty())
        color = info.pbr.albedo;
    else {
        Vector4 fgColor = getTextureAtUV(info.pbr.texture, r.uv.x, r.uv.y);
        float opacity = fgColor.w;
        float xi = fastRandom(r.randomState);
        if (xi < 1 - opacity) {
            return {-1, 0, 0};
        }
        color = {fgColor.x, fgColor.y, fgColor.z};
    }
    return color;
}


Vector3 nextEventEstimation(Ray & r) {
    //setup ray
    auto light = getLight(r);
    if(!light) return {0,0,0};

    auto xi = uniformSampleDisk(r);
    float xi1 = xi.x;
    float xi2 = xi.y; 
    float xi3 = 1 - xi1 - xi2;
    auto & tris = getTris();
    auto idx = getRandomTriangleFromObject(r, *light);
    if(idx >= tris.size()) return {};
    auto & tri = tris[idx];
    auto point = tri.vertices[1] * xi1 + tri.vertices[2] * xi2 + tri.vertices[0] * xi3;

    Vector3 origin = r.origin;
    Vector3 direction = normalized(point - origin); 
    Vector3 inv_direction = {1.0f / direction[0], 1.0f / direction[1], 1.0f / direction[2]};  
    Ray shadowRay = Ray{
        .origin = origin,
        .direction = direction,
        .inv_dir = inv_direction,
        .tmax = INFINITY
    };
    float cosSurface = dotProduct(r.normal, shadowRay.direction);
    if(cosSurface < EPS) return {};

    //check if light gets hit
    triangleIntersection(shadowRay, tri);
    float cosLight = -dotProduct(shadowRay.normal, shadowRay.direction);
    shadowRay.tmax -= EPS;
    float distance = shadowRay.tmax; 
    findIntersection(shadowRay);
    if(distance <= shadowRay.tmax - 0.01f || distance >= shadowRay.tmax + 0.01f) return {}; 
  
    float inv_square_distance = std::min(1.0f, (1.0f/(distance*distance)));
    //calculate color for hit light
    Material &lightMaterial = materials[shadowRay.materialIdx];
    Vector3 lightColor = getColorOfMaterial(shadowRay, lightMaterial);
    gammaCorrect(lightColor);
    
    if(lightColor[0] == -1) return {};
    lightColor = lightColor / 3.14f * lightMaterial.pbr.emmision * cosSurface * inv_square_distance * cosLight * light->surfaceArea * getLights().size();
    return lightColor;
}

//------- shader ----//
void reflectionShader(Ray &r) {
    Material &info = materials[r.materialIdx];
    r.origin = r.origin + r.direction * (r.tmax);
    r.direction =
        r.direction - 2.0f * dotProduct(r.direction, r.normal) * r.normal;
    auto randomDirection = randomCosineWeightedDirection(r);
    auto dir = randomDirection.x * r.tangent;
    dir += randomDirection.y * r.bitangent;
    dir += randomDirection.z * r.normal;
    normalize(dir);
    r.direction = r.direction * (1-info.pbr.roughness) + dir * info.pbr.roughness;
    normalize(r.direction);
    
    auto color = info.pbr.albedo;
    gammaCorrect(color);
    r.throughPut = r.throughPut * color;

    r.origin += r.direction * 0.01f;
    r.tmax = INFINITY;
    r.inv_dir[0] = 1.0f/r.direction[0];
    r.inv_dir[1] = 1.0f/r.direction[1];
    r.inv_dir[2] = 1.0f/r.direction[2];
    return;
}

void refractionShader(Ray &r) {
    int idx = r.materialIdx;
    Material &info = materials[idx];

    Vector3 normal = r.normal;
    float n1 = info.pbr.refractiveIdx1;
    float n2 = info.pbr.refractiveIdx2;
    float eta = n1 / n2;
    float cos = dotProduct(r.direction, r.normal);

    if (cos >= EPS) { // in object
        cos *= -1;
        eta = 1.0f / eta;
        normal = normal * -1;
        float tmp = n2;
        // n2 = n1;
        // n1 = tmp;
    }

    Vector3 refractDirection;
    float discriminator = 1.0f - (eta * eta) * (1.0f - (cos * cos));

    // internal relection
    float xi = fastRandom(r.randomState);
    float reflectance = calculateFresnelTerm(-cos, n1, n2);
    if (reflectance > 1)
        reflectance = 1;
    if (reflectance < 0)
        reflectance = 0;
    if (discriminator < EPS || xi < reflectance) {
        refractDirection = normalized(
            r.direction - 2.0f * dotProduct(r.direction, r.normal) * r.normal);
        // if(discriminator > eps)
        // r.throughPut *= reflectance * 1.0f / (reflectance);  // r * 1/r = 1
    } else {
        refractDirection = normalized(eta * (r.direction - cos * normal) -
                                      normal * sqrtf(discriminator + EPS));
        // r.throughPut *= 1 - reflectance;
        // r.throughPut *= 1.0f / (1-reflectance);
    }

    auto color = info.pbr.albedo;
    gammaCorrect(color);
    r.origin = r.origin + r.direction * (r.tmax) + refractDirection * EPS;
    r.throughPut = r.throughPut * color;
    r.direction = refractDirection;
    auto randomDirection = randomCosineWeightedDirection(r);
    auto dir = randomDirection.x * r.tangent;
    dir += randomDirection.y * r.bitangent;
    dir += randomDirection.z * r.normal;
    normalize(dir);
    r.direction = r.direction * (1-info.pbr.roughness) + dir * info.pbr.roughness;
    normalize(r.direction);
    r.tmax = INFINITY;
    r.inv_dir[0] = 1.0f/r.direction[0];
    r.inv_dir[1] = 1.0f/r.direction[1];
    r.inv_dir[2] = 1.0f/r.direction[2];
    // r.colorMask = r.colorMask * info->color;

    return;
}


void lambertShader(Ray &r) {
    int idx = r.materialIdx;
    Material &info = materials[idx];
    float cos = dotProduct(r.normal, r.direction);
    Vector3 color = getColorOfMaterial(r, info);
    if(color[0] == -1) {
        r.origin = r.origin + r.direction * (r.tmax + 0.01f);
        r.tmax = INFINITY;
        return;
    }
    gammaCorrect(color);

    // reset for next bounce
    r.origin = r.origin + r.direction * r.tmax;
    
    auto randomDir = (randomCosineWeightedDirection(r));
    r.direction = randomDir.x * r.tangent + randomDir.y * r.bitangent + randomDir.z * r.normal;
    normalize(r.direction);
    
    r.origin += r.direction * EPS;
    r.tmax = INFINITY;
    r.inv_dir[0] = 1.0f/r.direction[0];
    r.inv_dir[1] = 1.0f/r.direction[1];
    r.inv_dir[2] = 1.0f/r.direction[2];
    r.throughPut = r.throughPut * (color);
   
    //next event estimation
    Vector3 lightColor{};
    if(nee) lightColor = nextEventEstimation(r);
    r.light = r.light + lightColor * r.throughPut;
    
    return;
}

u32 randomState;
Vector3 shade(Ray &r) {
    Vector3 black{0.0f, 0.0f, 0.0f};
    int idx = r.materialIdx;
    auto & mat = materials[idx];
    if(r.tmax == INFINITY) return black;

    if(primaryOnly) {
        r.terminated = true;
        r.light=  mat.pbr.albedo;
    }
    float xi = fastRandom(r.randomState);
    
    Vector3 normal = r.normal;
    if (mat.pbr.normal.data.size() > 0) {
        Vector4 const normalColor = getTextureAtUV(mat.pbr.normal, r.uv.x, r.uv.y);
        Vector3 const textureNormal = Vector3{2.0f * normalColor.x, 2.0f * normalColor.y, 2.0f * normalColor.z} - Vector3{1, 1, 1};
        normal = textureNormal.x * r.tangent + textureNormal.y * r.bitangent + textureNormal.z * r.normal;
        normalize(normal);
    }
    r.normal = normal;

    auto flag = r.rayFLAG;

    Vector3 lightColor = {};
    if(xi < mat.weights.lambert) {
        lambertShader(r);
        r.rayFLAG = OTHER;
    } else if (xi < (mat.weights.lambert + mat.weights.reflection)) {
        reflectionShader(r);
        r.rayFLAG = REFLECTION_RAY;
    } else {
        refractionShader(r);
        r.rayFLAG = REFLECTION_RAY;
    }
    //handle if material is emmisive
    //ignore when nee is active and this is a difuse ray
    if(!nee || (nee)) r.light = r.light + mat.pbr.emmision * r.throughPut;
    return {};
}

