#include "object.h"
#include "common.h"
#include "scene/scene.h"
#include "shader/shader.h"
#include "triangle.h"
#include "primitive.h"
#include "types/bvh.h"
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
    if(getIntersectMode() != ALL) {
        float l = ray.tmax;
        ray.interSectionAS++;
        findBVHIntesection(ray, primitive.root, true);
        hit |= ray.tmax != l;
    }
    
    if(getIntersectMode() == ALL) {
        for (int i = primitive.startIdx; i < primitive.endIdx; ++i) {
            ray.interSectionTests++;
            hit |= triangleIntersection(ray, trisBuffer[i]);
        }
    }
    if(hit) {
       ray.materialIdx = primitive.materialIdx; 
    }
    return hit;
}
void loadObject(const char *fileName, Vector3 position, Vector3 size,
                  int materialIdx) {
    auto & objectBuffer = getObjects();
    auto & indicieBuffer = getIndicies();
    auto & trisBuffer = getTris();
    // load the triangles and normals
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                          fileName)) {
        printf("error: %s %s\n", warn.c_str(), err.c_str());
        exit(1);
    }

    int firstMatIdx = getMaterials()->size();

    for (const auto & material : materials) {
        Material m {
            .color = {material.diffuse[0], material.diffuse[1], material.diffuse[2]},
            .shaderFlag = SHADOWSHADER,
            .name = std::string(material.name.c_str()),
        };
        auto emit = material.emission;
        if(emit[0] != 0 || emit[1] != 0 || emit[2] != 0) {
            m.shaderFlag = EMITSHADER;
            Vector3 c = {emit[0], emit[1], emit[2]};
            float ma = max(c);
            float i = 1;
            if(ma >= 1) {
                c[0]/=ma;
                c[1]/=ma;
                c[2]/=ma;
                i=ma;
            }
            m.intensity = i;
            m.color = c;
        }
        int idx = addMaterial(m);
        if(!material.diffuse_texname.empty()) loadTexture(getMaterial(idx)->texture, material.diffuse_texname);
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
            
            int idx = trisBuffer.size();
            trisBuffer.push_back(createTriangle(verts[0], verts[1], verts[2],
                                                   normals[0], normals[1],
                                                   normals[2], uvs[0], uvs[1], uvs[2], matIdx));
            if(i == 0)startIdx = idx;
            endIdx = std::max(endIdx, idx + 1);
        }

        
        //boundingbox
        Vector3 min, max{};
        Vector3 tmin, tmax{};
        for (int i = startIdx; i < endIdx; i++) {
            auto & vertice = trisBuffer[i];
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
            .materialIdx = matIdx,
            .root = constructBVH(startIdx, endIdx, -1, true),
            .active = true,
            .name = std::string(shape.name.c_str()),
        };
        objectBuffer.push_back(o);
    }
}
