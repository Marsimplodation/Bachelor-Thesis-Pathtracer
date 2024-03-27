#include "object.h"
#include "common.h"
#include "scene/scene.h"
#include "cube.h"
#include "triangle.h"
#include "types/bvh.h"
#include <cstdlib>
#include <string>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

bool findIntersection(Ray &ray, Object &primitive) {
    Ray bRay = ray;
    if (!findIntersection(bRay, primitive.boundingBox))
        return false;
    
    if(getIntersectMode() == BVH) {
        findBVHIntesection(ray, &primitive.root, true);
        return ray.hit;
    }

    auto vertices = getObjectBufferAtIdx(primitive.startIdx);
    bool hit = false;
    for (int i = 0; i < primitive.endIdx - primitive.startIdx; i++) {
        ray.interSectionTests++;
        hit |= findIntersection(ray, vertices[i]);
    }
    return hit;
}
Object loadObject(const char *fileName, Vector3 position, Vector3 size,
                  void *shaderInfo) {
    auto buffer = getObjectBuffer();
    int startIdx = buffer->count;
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
    
    Vector3 verts[3];
    Vector3 normals[3];
    tinyobj::index_t idxs[3];
    for (const auto &shape : shapes) {
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
            }
            
            addToPrimitiveContainer(*buffer,
                                    createTriangle(verts[0], verts[1], verts[2],
                                                   normals[0], normals[1],
                                                   normals[2], shaderInfo));
        }
    }

    int endIdx = buffer->count;
    
    //boundingbox
    auto vertices = getObjectBufferAtIdx(startIdx);
    Vector3 min, max{};
    Vector3 tmin, tmax{};
    for (int i = 0; i < endIdx - startIdx; i++) {
        tmin = minBounds(vertices[i]);
        tmax = maxBounds(vertices[i]);
        if (tmin.x < min.x) min.x = tmin.x;
        if (tmin.y < min.y) min.y = tmin.y;
        if (tmin.z < min.z) min.z = tmin.z;
        if (tmax.x > max.x) max.x = tmax.x;
        if (tmax.y > max.y) max.y = tmax.y;
        if (tmax.z > max.z) max.z = tmax.z;
    }

    // Calculate the center and size of the bounding box
    Vector3 center = {
        (min.x + max.x) * 0.5f,
        (min.y + max.y) * 0.5f,
        (min.z + max.z) * 0.5f
    };
    
    Vector3 ssize = {
        max.x - min.x,
        max.y - min.y,
        max.z - min.z
    };
    return {
        .type = OBJECT,
        .startIdx = startIdx,
        .endIdx = endIdx,
        .boundingBox =  {.center = center, .size=ssize, .shaderInfo=shaderInfo},
        .shaderInfo = shaderInfo,
        .root = constructBVH(startIdx, endIdx, true),
    };
}

Vector3 minBounds(Object &primitive) {
    return minBounds(primitive.boundingBox);
}
Vector3 maxBounds(Object &primitive) {
    return maxBounds(primitive.boundingBox);
}
