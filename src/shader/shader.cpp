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

u32 randomState;
Vector3 shade(Ray &r) {
    Vector3 black{0.0f, 0.0f, 0.0f};
    if (r.tmax == INFINITY)
        return black;
    int idx = r.materialIdx;
    switch (materials[idx].shaderFlag) {
    case EMITSHADER:
        return emitShader(r);
    case MIRRORSHADER:
        return mirrorShader(r);
    case SHADOWSHADER:
        return shadowShader(r);
    case REFRACTSHADER:
        return refractionShader(r);
    case EDGESHADER:
        return edgeShader(r);
    default:
        return r.normal;
    }
}

Vector3 mirrorShader(Ray &r) {
    r.origin = r.origin + r.direction * (r.tmax);
    r.direction =
        r.direction - 2.0f * dotProduct(r.direction, r.normal) * r.normal;
    r.origin += r.direction * 0.01f;
    r.tmax = INFINITY;
    r.inv_dir[0] = 1.0f/r.direction[0];
    r.inv_dir[1] = 1.0f/r.direction[1];
    r.inv_dir[2] = 1.0f/r.direction[2];
    return {};
}

Vector3 emitShader(Ray &r) {
    int idx = r.materialIdx;
    Material &info = materials[idx];

    bool t = r.terminated;
    r.terminated = true;
    if (info.texture.data.empty())
        return info.color * info.intensity * r.throughPut;

    Vector4 fgColor = getTextureAtUV(info.texture, r.uv.x, r.uv.y);
    float opacity = fgColor.w;
    float xi = fastRandom(r.randomState);
    if (xi < 1 - opacity) {
        r.terminated = t;
        r.origin = r.origin + r.direction * (r.tmax + 0.01f);
        r.tmax = INFINITY;
        return {};
    }
    Vector3 color = {fgColor.x, fgColor.y, fgColor.z};
    gammaCorrect(color);
    return (color * info.intensity * r.throughPut);
}

Vector3 refractionShader(Ray &r) {
    int idx = r.materialIdx;
    Material &info = materials[idx];

    Vector3 normal = r.normal;
    float n1 = info.refractiveIdx1;
    float n2 = info.refractiveIdx2;
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

    auto color = info.color;
    gammaCorrect(color);
    r.origin = r.origin + r.direction * (r.tmax) + refractDirection * EPS;
    r.throughPut = r.throughPut * color;
    r.direction = refractDirection;
    r.tmax = INFINITY;
    r.inv_dir[0] = 1.0f/r.direction[0];
    r.inv_dir[1] = 1.0f/r.direction[1];
    r.inv_dir[2] = 1.0f/r.direction[2];
    // r.colorMask = r.colorMask * info->color;

    return {};
}

Vector3 edgeShader(Ray &r) {
    int idx = r.materialIdx;
    Material &info = materials[idx];
    Vector3 fgColor = info.color;

    float cos = dotProduct(r.normal, r.direction);
    float edge = 1.0f;
    if (fabs(cos) > info.intensity)
        edge = .0f;
    if (edge) {
        r.terminated = true;
        return info.color2;
    }

    Vector3 color = {0, 0, 0};
    if (info.texture.data.empty())
        color = info.color;
    else {
        Vector4 fgColor = getTextureAtUV(info.texture, r.uv.x, r.uv.y);
        float opacity = fgColor.w;
        float xi = fastRandom(r.randomState);
        if (xi < 1 - opacity) {
            r.origin = r.origin + r.direction * (r.tmax + 0.01f);
            r.tmax = INFINITY;
            return {0, 0, 0};
        }
        color = {fgColor.x, fgColor.y, fgColor.z};
    }
    r.terminated = true;
    gammaCorrect(color);
    return color;

    // reset for next bounce
    r.throughPut = r.throughPut * color * fabsf(cos);
    r.origin = r.origin + r.direction * r.tmax;
    r.direction = randomCosineWeightedDirection(r);
    r.origin += r.direction * EPS;
    r.tmax = INFINITY;
    r.inv_dir[0] = 1.0f/r.direction[0];
    r.inv_dir[1] = 1.0f/r.direction[1];
    r.inv_dir[2] = 1.0f/r.direction[2];
    return {};
}

Vector3 shadowShader(Ray &r) {
    int idx = r.materialIdx;
    Material &info = materials[idx];
    float cos = dotProduct(r.normal, r.direction);
    Vector3 color = {0, 0, 0};
    if (info.texture.data.empty())
        color = info.color;
    else {
        Vector4 fgColor = getTextureAtUV(info.texture, r.uv.x, r.uv.y);
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
