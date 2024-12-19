#ifndef COMMON_H
#define COMMON_H
#include <float.h>
#define u32 unsigned int
#define u64 uint64_t
#define EPS 0.00001f
#ifndef INFINITY
#define INFINITY FLT_MAX
#endif // !INFINITY
float fastRandom(u32 & seed);
u32 hashCoords(int x, int y);

#include <glm/vec4.hpp>
struct Vertex {
     glm::vec4 position;
     glm::vec4 color;
};

#endif
