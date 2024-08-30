#include "object.h"
#include "common.h"
#include "scene/scene.h"
#include "shader/shader.h"
#include "triangle.h"
#include "primitive.h"
#include "types/bvh.h"
#include "types/lightFieldGrid.h"
#include "types/texture.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

bool objectIntersection(Ray &ray, Object &primitive) {
    if(!primitive.active) return false;
    auto & trisBuffer = getTris();
    //if (!findIntersection(ray, primitive.boundingBox))
    //    return false;
    
    bool hit = false;
    
    if(getIntersectMode() == ALL) {
        for (int i = primitive.startIdx; i < primitive.endIdx; ++i) {
            ray.interSectionTests++;
            hit |= triangleIntersection(ray, trisBuffer[i]);
        }
    }
    if(getIntersectMode() == BVH) {
        findBVHIntesection(ray, primitive.root);
    }
    
    if(getIntersectMode() == GRID) {
        int axis = 0;
        float maxDelta = 0.0f;
        float f0 = std::abs(ray.direction[0]);
        float f1 = std::abs(ray.direction[1]);
        float f2 = std::abs(ray.direction[2]);

        if (f0 > maxDelta) {
            maxDelta = f0;
            axis = 0;
        }
        if (f1 > maxDelta) {
            maxDelta = f1;
            axis = 1;
        }
        if (f2 > maxDelta) {
            maxDelta = f2;
            axis = 2;
        }
        intersectGrid(ray, primitive.GridIdx[axis]);
    }
    return hit;
}
void loadObject(const std::string fileName, Vector3 position, Vector3 size,
                  int materialIdx) {
    auto & objectBuffer = getObjects();
    auto & indicieBuffer = getIndicies();
    auto & trisBuffer = getTris();
    // load the triangles and normals
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    std::string base_dir = fileName.substr(0, fileName.find_last_of('/'));
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                          fileName.c_str(), base_dir.c_str())) {
        printf("error: %s %s\n", warn.c_str(), err.c_str());
        exit(1);
    }

    int firstMatIdx = getMaterials()->size();

    for (const auto & material : materials) {
        Material m {
            .pbr = {
                .albedo = {material.diffuse[0], material.diffuse[1], material.diffuse[2]},
            },
            .name = std::string(material.name.c_str()),
        };
        auto emit = material.emission;
        if(emit[0] != 0 || emit[1] != 0 || emit[2] != 0) {
            Vector3 c = {emit[0], emit[1], emit[2]};
            float ma = max(c);
            float i = 1;
            if(ma >= 1) {
                c[0]/=ma;
                c[1]/=ma;
                c[2]/=ma;
                i=ma;
            }
            m.pbr.emmision = i;
            m.pbr.albedo = c;
        }
        int idx = addMaterial(m);

        bool isAbsolute = material.diffuse_texname.front() == '/';
        std::string texture = isAbsolute? material.diffuse_texname:base_dir + "/" + material.diffuse_texname;
    
        if(!material.diffuse_texname.empty()) loadTexture(getMaterial(idx)->pbr.texture, texture.c_str());
        
        isAbsolute = material.bump_texname.front() == '/';
        texture = isAbsolute? material.diffuse_texname:base_dir + "/" + material.bump_texname;
    
        if(!material.bump_texname.empty()) loadTexture(getMaterial(idx)->pbr.normal, texture.c_str());
        printf("loaded material %s\n", m.name.c_str());
    }
    
   
    for (const auto &shape : shapes) {
        int startIdx = trisBuffer.size();
        int endIdx = trisBuffer.size();
        printf("loaded: %s\n", shape.name.c_str());
        Vector3 verts[3];
        Vector3 normals[3];
        Vector2 uvs[3];
        tinyobj::index_t idxs[3];
        int matIdx = shape.mesh.material_ids[0];
        if(matIdx >= 0)matIdx += firstMatIdx;
        else matIdx = materialIdx;
        std::vector<u32> objectMats(0);
        std::vector<bool> matUsed(materials.size(), false);
        for (size_t i = 0; i < shape.mesh.indices.size(); i += 3) {
            idxs[0] = shape.mesh.indices[i];
            idxs[1] = shape.mesh.indices[i+1];
            idxs[2] = shape.mesh.indices[i+2];

            for(int j = 0; j <3; j++) {
                verts[j]={
                    attrib.vertices[3 * idxs[j].vertex_index + 0] * size.x + position.x,
                    attrib.vertices[3 * idxs[j].vertex_index + 1] * size.y + position.y,
                    attrib.vertices[3 * idxs[j].vertex_index + 2] * size.z + position.z,
                };
                normals[j]={
                    attrib.normals[3 * idxs[j].normal_index + 0],
                    attrib.normals[3 * idxs[j].normal_index + 1],
                    attrib.normals[3 * idxs[j].normal_index + 2],
                };
                uvs[j]={
                    attrib.texcoords[2 * idxs[j].texcoord_index + 0],
                    1.0f-attrib.texcoords[2 * idxs[j].texcoord_index + 1],
                };


            }
            int matIdxTri = shape.mesh.material_ids[i / 3];  // Material ID for this specific triangle
            if (matIdxTri >= 0) {
                if(!matUsed[matIdxTri]) {
                    matUsed[matIdxTri] = true;
                    objectMats.push_back(matIdxTri + firstMatIdx);
                }
                matIdxTri += firstMatIdx;
            } else {
                matIdxTri = matIdx;  // Default material index if no material assigned
            }
            
            int idx = trisBuffer.size();
            trisBuffer.push_back(createTriangle(verts[0], verts[1], verts[2],
                                                   normals[0], normals[1],
                                                   normals[2], uvs[0], uvs[1], uvs[2], matIdxTri));
            if(i == 0)startIdx = idx;
            endIdx = std::max(endIdx, idx + 1);
        }

        //boundingbox
        Vector3 min{INFINITY, INFINITY, INFINITY};
        Vector3 max{-INFINITY, -INFINITY, -INFINITY};
        float sA = 0.0f;
        Vector3 tmin, tmax{};
        for (int i = startIdx; i < endIdx; i++) {
            auto & vertice = trisBuffer[i];
            sA += calculateTriangleSurfaceArea(vertice);
            tmin = minBounds(vertice);
            tmax = maxBounds(vertice);
            if (tmin.x < min.x) min.x = tmin.x;
            if (tmin.y < min.y) min.y = tmin.y;
            if (tmin.z < min.z) min.z = tmin.z;
            if (tmax.x > max.x) max.x = tmax.x;
            if (tmax.y > max.y) max.y = tmax.y;
            if (tmax.z > max.z) max.z = tmax.z;
        }
        Object o{
            .type = OBJECT,
            .startIdx = startIdx,
            .endIdx = endIdx,
            .boundingBox =  {.min = min, .max=max},
            .active = true,
            .surfaceArea = sA,
            .name = std::string(shape.name.c_str()),
            .materials = objectMats,
        };
        objectBuffer.push_back(o);
    }
}

u32 getRandomTriangleFromObject(Ray & ray, Object & primitive) {
    u32 range = primitive.endIdx - primitive.startIdx;
    u32 randomIdx = primitive.startIdx + std::floor(fastRandom(ray.randomState) * range);
    return randomIdx;
}
