#include "VkRenderer.h"
#include "glm/ext/vector_float3.hpp"
#include <functional>
#include <string>
#include <tiny_obj_loader.h>
#include <unordered_map>
const std::string MODEL_PATH = "scenes/test2.obj";
const std::string BASE_DIR = "scenes/";
#define GAMMA 2.2f

bool Vertex::operator==(const Vertex& other) const {
    return position == other.position 
            && materialIdx == other.materialIdx 
            && normal == other.normal;
}
namespace std {
    inline void hash_combine(std::size_t& seed, std::size_t hash) {
        // A simple and effective hash combination function
        seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    template <> struct hash<glm::vec4> {
            std::size_t operator()(const glm::vec4& v) const {
                std::size_t seed = 0;
                std::size_t h1 = std::hash<float>()(v.x);
                std::size_t h2 = std::hash<float>()(v.y);
                std::size_t h3 = std::hash<float>()(v.z);
                std::size_t h4 = std::hash<float>()(v.w);
                // Combine the hashes of x, y, z, and w
                // Combine the hashes
                hash_combine(seed, h1);
                hash_combine(seed, h2);
                hash_combine(seed, h3);
                hash_combine(seed, h4);

                return seed;
            }
        };
    template<> struct hash<Vertex> {
    size_t operator()(Vertex const& vertex) const {
        std::size_t seed = 0;

        // Hash individual fields
        std::size_t h1 = std::hash<glm::vec4>()(vertex.position);
        std::size_t h3 = std::hash<glm::vec4>()(vertex.normal);
        std::size_t h4 = std::hash<u32>()(vertex.materialIdx);

        // Combine the hashes
        hash_combine(seed, h1);
        hash_combine(seed, h3);
        hash_combine(seed, h4);

        return seed;
    }};
}


void VkRenderer::loadGeometry() {    
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> obj_materials;
    std::string warn, err;
    
    vertices = std::vector<Vertex>();
    indices = std::vector<u32>();
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    if (!tinyobj::LoadObj(&attrib, &shapes, &obj_materials, &warn, &err, MODEL_PATH.c_str(), BASE_DIR.c_str())) {
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
        materials.push_back(m);
        materialNames.push_back(material.name);
    }

    u32 count = 0;
    for (const auto& shape : shapes) {
        u32 faceIndex = 0;
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

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
            faceIndex++;
            count++;
        }
    }
    printf("vertices: %d deduplicated: %zu\n", count, vertices.size());
}
