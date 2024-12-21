#include "VkRenderer.h"
#include "glm/ext/vector_float3.hpp"
#include <tiny_obj_loader.h>
const std::string MODEL_PATH = "scenes/test.obj";
const std::string BASE_DIR = "scenes/";
#define GAMMA 2.2f
void VkRenderer::loadGeometry() {    
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    vertices = std::vector<Vertex>();
    indices = std::vector<u32>();
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str(), BASE_DIR.c_str())) {
        throw std::runtime_error(warn + err);
    }
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


            vertices.push_back(vertex);
            indices.push_back(indices.size());
            faceIndex++;
        }
    }
}
