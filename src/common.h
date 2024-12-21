#ifndef COMMON_H
#define COMMON_H
#include <float.h>
#define u32 unsigned int
#define u8 char 
#define u64 uint64_t
#define EPS 0.00001f
#ifndef INFINITY
#define INFINITY FLT_MAX
#endif // !INFINITY
float fastRandom(u32 & seed);
u32 hashCoords(int x, int y);

//glm imports
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct Vertex {
     glm::vec4 position;
     glm::vec4 normal;
     glm::vec4 color;
     float emission;
     u32 shaderFlag;
     u32 padding[2];
     bool operator==(const Vertex& other) const;
};


#endif
