#ifndef COMMON_H
#define COMMON_H
#include <cstdint>
#define u32 unsigned int
#define u64 uint64_t
#define EPS 0.00001f
#define INFINITY MAX_FLOAT
float fastRandom(u32 & seed);
u32 hashCoords(int x, int y);
enum IntersectMode{ALL, BVH, GRID, HYBRID};
IntersectMode getIntersectMode();
void setIntersectMode(int i);
#endif
