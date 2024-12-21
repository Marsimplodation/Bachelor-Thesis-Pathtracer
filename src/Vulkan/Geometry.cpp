#include "VkRenderer.h"
#include "glm/ext/vector_float3.hpp"
#include <functional>
#include <tiny_obj_loader.h>
#include <unordered_map>
const std::string MODEL_PATH = "scenes/test.obj";
const std::string BASE_DIR = "scenes/";
#define GAMMA 2.2f

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
        std::size_t h2 = std::hash<glm::vec4>()(vertex.color);
        std::size_t h3 = std::hash<glm::vec4>()(vertex.normal);
        std::size_t h4 = std::hash<float>()(vertex.emission);

        // Combine the hashes
        hash_combine(seed, h1);
        hash_combine(seed, h2);
        hash_combine(seed, h3);
        hash_combine(seed, h4);

        return seed;
    }};
}


void VkRenderer::loadGeometry() {    
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    
    vertices = std::vector<Vertex>();
    indices = std::vector<u32>();
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str(), BASE_DIR.c_str())) {
        throw std::runtime_error(warn + err);
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

            // Check if the material ID is valid
            if (material_id >= 0 && material_id < materials.size()) {
                const auto& material = materials[material_id];

                // Use the diffuse color (Kd) as the albedo
                vertex.color = glm::vec4{
                    material.diffuse[0], // Red
                    material.diffuse[1], // Green
                    material.diffuse[2], // Blue
                    1.0f                 // Alpha
                };
                vertex.color[0] = std::powf(vertex.color[0], GAMMA);
                vertex.color[1] = std::powf(vertex.color[1], GAMMA);
                vertex.color[2] = std::powf(vertex.color[2], GAMMA);
                glm::vec3 emission = glm::vec3{material.emission[0], material.emission[1], material.emission[2]};
                vertex.emission = emission[0];
            } else {
                vertex.emission = 0.0f;
                // Default color if no material is assigned
                vertex.color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}; // White
            }
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
