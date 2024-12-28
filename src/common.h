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



#endif
