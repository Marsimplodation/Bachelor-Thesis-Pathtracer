#include "common.h"

u32 hashCoords(int x, int y) {
    return (static_cast<u32>(x) << 16 | y);
}

float fastRandom(u32 & seed) {
    seed ^= seed << 13; //xor with const;
    seed ^= seed >> 17; //xor with const;
    seed ^= seed << 5; //xor with const;
    return static_cast<float>(seed % 1000)/1000.0f;
}
