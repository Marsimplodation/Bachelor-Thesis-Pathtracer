#include "common.h"
#include <cstdint>

namespace {
    IntersectMode mode = TWO_PLANE;
}

u32 hashCoords(int x, int y) {
    return (static_cast<u32>(x) << 16 | y);
}

IntersectMode getIntersectMode() {return  mode;}

void setIntersectMode(int i) {mode = (IntersectMode)i;}
float fastRandom(u32 & seed) {
    //using PCG HASH
    u32 state = seed * 747796405u + 2891336453u;
    u32 word = ((state >> ((state >> 28u) + 4u))^state) * 277803737u;
    seed = (word >> 22u) ^ word;
    return (float)seed / (float)UINT32_MAX;
}
