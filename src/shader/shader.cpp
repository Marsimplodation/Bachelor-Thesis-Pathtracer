#include "shader.h"
#include "common.h"
#include "scene/scene.h"
#include "types/texture.h"
#include "types/vector.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
namespace {
#define GAMMA 2.2f
std::vector<Material> materials;
} // namespace

Material *getMaterial(int idx) { return &materials[idx]; }

std::vector<Material> *getMaterials() { return &materials; }

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

//------- shader ----//
Vector3 reflectionShader(Ray &r) {
    Material &info = materials[r.materialIdx];
    r.origin = r.origin + r.direction * (r.tmax);
    r.direction =
        r.direction - 2.0f * dotProduct(r.direction, r.normal) * r.normal;
    auto dir = randomCosineWeightedDirection(r);
    r.direction = r.direction * (1-info.pbr.roughness) + dir * info.pbr.roughness;
    normalize(r.direction);

    r.origin += r.direction * 0.01f;
    r.tmax = INFINITY;
    r.inv_dir[0] = 1.0f/r.direction[0];
    r.inv_dir[1] = 1.0f/r.direction[1];
    r.inv_dir[2] = 1.0f/r.direction[2];
    return {};
}

Vector3 refractionShader(Ray &r) {
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
    auto dir = randomCosineWeightedDirection(r);
    r.direction = r.direction * (1-info.pbr.roughness) + dir * info.pbr.roughness;
    normalize(r.direction);
    r.tmax = INFINITY;
    r.inv_dir[0] = 1.0f/r.direction[0];
    r.inv_dir[1] = 1.0f/r.direction[1];
    r.inv_dir[2] = 1.0f/r.direction[2];
    // r.colorMask = r.colorMask * info->color;

    return {};
}


Vector3 lambertShader(Ray &r) {
    int idx = r.materialIdx;
    Material &info = materials[idx];
    float cos = dotProduct(r.normal, r.direction);
    Vector3 color = {0, 0, 0};
    if (info.pbr.texture.data.empty())
        color = info.pbr.albedo;
    else {
        Vector4 fgColor = getTextureAtUV(info.pbr.texture, r.uv.x, r.uv.y);
        float opacity = fgColor.w;
        float xi = fastRandom(r.randomState);
        if (xi < 1 - opacity) {
            r.origin = r.origin + r.direction * (r.tmax + 0.01f);
            r.tmax = INFINITY;
            return {};
        }
        color = {fgColor.x, fgColor.y, fgColor.z};
    }
    gammaCorrect(color);

    // reset for next bounce
    r.throughPut = r.throughPut * color;
    r.origin = r.origin + r.direction * r.tmax;
    r.direction = randomCosineWeightedDirection(r);
    r.origin += r.direction * EPS;
    r.tmax = INFINITY;
    r.inv_dir[0] = 1.0f/r.direction[0];
    r.inv_dir[1] = 1.0f/r.direction[1];
    r.inv_dir[2] = 1.0f/r.direction[2];
    return {};
}

u32 randomState;
Vector3 shade(Ray &r) {
    Vector3 black{0.0f, 0.0f, 0.0f};
    if (r.tmax == INFINITY)
        return black;
    int idx = r.materialIdx;
    auto & mat = materials[idx];
    float xi = fastRandom(r.randomState);

    if(xi < mat.weights.lambert) {
        lambertShader(r);
    } else if (xi < (mat.weights.lambert + mat.weights.reflection)) {
        reflectionShader(r);
    } else {
        refractionShader(r);
    }

    return r.throughPut * mat.pbr.emmision;
}

