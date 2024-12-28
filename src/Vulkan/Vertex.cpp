#include "VkRenderer.h"

bool Vertex::operator==(const Vertex& other) const {
    return position == other.position 
            && materialIdx == other.materialIdx 
            && uv == other.uv 
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
    template <> struct hash<glm::vec2> {
            std::size_t operator()(const glm::vec2& v) const {
                std::size_t seed = 0;
                std::size_t h1 = std::hash<float>()(v.x);
                std::size_t h2 = std::hash<float>()(v.y);
                // Combine the hashes of x, y, z, and w
                // Combine the hashes
                hash_combine(seed, h1);
                hash_combine(seed, h2);

                return seed;
            }
    };

    size_t hash<Vertex>::operator()(Vertex const& vertex) const {
        std::size_t seed = 0;

        // Hash individual fields
        std::size_t h1 = std::hash<glm::vec4>()(vertex.position);
        std::size_t h2 = std::hash<glm::vec2>()(vertex.uv);
        std::size_t h3 = std::hash<glm::vec4>()(vertex.normal);
        std::size_t h4 = std::hash<u32>()(vertex.materialIdx);

        // Combine the hashes
        hash_combine(seed, h1);
        hash_combine(seed, h2);
        hash_combine(seed, h3);
        hash_combine(seed, h4);

        return seed;
    }
}
