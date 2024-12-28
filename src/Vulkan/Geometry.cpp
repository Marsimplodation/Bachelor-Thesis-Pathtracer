#include "VkRenderer.h"
#include "glm/detail/qualifier.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include <functional>
#include <string>
#include <tiny_obj_loader.h>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const std::string MODEL_PATH = "scenes/sponza/test.obj";
#define GAMMA 2.2f


//returns texture data as
//0 : offset
//1 : width
//2 : height
//3 : 
glm::vec4 loadTexture(std::vector<glm::vec4> & textureAtlas, std::string path) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    u32 offset = textureAtlas.size();
    if(!pixels) {
        printf("could not load: %s\n", path.c_str());
        return glm::vec4(-1,0,0,0);
    }
    // Clear any existing data in the texture

    // Copy pixel data to the texture
    for (int y = 0; y < texHeight; ++y) {
        for (int x = 0; x < texWidth; ++x) {
            int pixelIndex = (y * texWidth + x) * 4; // RGBA channels
            float r = pixels[pixelIndex] / 255.0f;
            float g = pixels[pixelIndex + 1] / 255.0f;
            float b = pixels[pixelIndex + 2] / 255.0f;
            float a = pixels[pixelIndex + 3] / 255.0f;
            r = std::powf(r, GAMMA);
            g  = std::powf(g, GAMMA);
            b = std::powf(b, GAMMA);
            textureAtlas.push_back({ r, g, b, a});
        }
    }
    
    // Free stb_image allocated memory
    stbi_image_free(pixels);
    printf("loaded texture %s\n", path.c_str());
    return glm::vec4(offset, texWidth, texHeight, 0.0);
}

void VkRenderer::loadGeometry() {    
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> obj_materials;
    std::string warn, err;
    
    vertices = std::vector<Vertex>();
    indices = std::vector<u32>();
    emissiveTriangles = std::vector<u32>();
    textureAtlas = std::vector<glm::vec4>();
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    std::string base_dir = MODEL_PATH.substr(0, MODEL_PATH.find_last_of('/'));

    if (!tinyobj::LoadObj(&attrib, &shapes, &obj_materials, &warn, &err, MODEL_PATH.c_str(), base_dir.c_str())) {
        throw std::runtime_error(warn + err);
    }

    //--load materials first---/
    materials = std::vector<Material>();
    materialNames = std::vector<std::string>();

    for (const auto & material : obj_materials) {
        Material m {
            .color = {material.diffuse[0], material.diffuse[1], material.diffuse[2],1.0f},
        };
        m.color[0] = std::powf(m.color[0], GAMMA);
        m.color[1] = std::powf(m.color[1], GAMMA);
        m.color[2] = std::powf(m.color[2], GAMMA);
        auto emit = material.emission;
        if(emit[0] != 0 || emit[1] != 0 || emit[2] != 0) {
            glm::vec3 c = {emit[0], emit[1], emit[2]};
            float ma = fmaxf(emit[0], fmaxf(emit[1], emit[2]));
            float i = 1;
            if(ma >= 1) {
                c[0]/=ma;
                c[1]/=ma;
                c[2]/=ma;
                i=ma;
            }
            m.emission = i;
        }

        //load texture
        //
        bool isAbsolute = material.diffuse_texname.front() == '/';
        bool isAbsoluteNormal = material.bump_texname.front() == '/';
        std::string texture = isAbsolute? material.diffuse_texname : base_dir + "/" + material.diffuse_texname;
        std::string textureNormal = isAbsoluteNormal? material.bump_texname : base_dir + "/" + material.bump_texname;

        if(!material.diffuse_texname.empty()) {
            m.textureData = loadTexture(textureAtlas, texture.c_str());
        } else m.textureData[0] = -1;
        
        if(!material.bump_texname.empty()) {
            m.normalMapData = loadTexture(textureAtlas, textureNormal.c_str());
        } else m.normalMapData[0] = -1;
        materials.push_back(m);
        materialNames.push_back(material.name);
    }

    u32 count = 0;
    for (const auto& shape : shapes) {
        u32 faceIndex = 0;
        printf("start loading %s\n", shape.name.c_str());
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};
            vertex.position = glm::vec4{
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2],
                1.0f,
            };
            // Retrieve material index
            int material_id = shape.mesh.material_ids[faceIndex/3];
            vertex.materialIdx = material_id;

            // Add normals
            vertex.normal = glm::vec4{
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2],
                1.0f
            };

            vertex.uv = glm::vec2{
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
            };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
            faceIndex++;
            count++;
        }
        printf("loaded %s\n", shape.name.c_str());
    }
    for (int i = 0; i < indices.size(); i+=3) {
        Vertex & v0 = vertices[indices[i]];
        if(materials[v0.materialIdx].emission > 0.0f) {
            emissiveTriangles.push_back(i);
        }
    }
    printf("vertices: %d deduplicated: %zu\n", count, vertices.size());
}
