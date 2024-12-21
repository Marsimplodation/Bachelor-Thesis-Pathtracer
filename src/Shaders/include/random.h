#ifndef RANDOM_H
#define RANDOM_H
#define UINT_MAX 0xFFFFFFFFu

float fastRandom(inout uint seed) {
    // Using PCG Hash
    uint state = seed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    seed = (word >> 22u) ^ word;
    return float(seed) /float(UINT_MAX);
}

uint hashCoords(int x, int y) {
    return (uint(x) << 16) | uint(y);
}

vec3 randomCosineWeightedDirection(inout uint seed) {
    vec3 direction;
    float theta = fastRandom(seed);
    float sinTheta = sqrt(theta);
    float phi = 2 * 3.14f * (fastRandom(seed));
    direction.x = sinTheta * cos(phi);
    direction.y = sinTheta * sin(phi);
    direction.z = sqrt(max(0.0f, 1.0f - direction.x * direction.x - direction.y * direction.y));
    return direction;
}


#endif // !RANDOM_H
