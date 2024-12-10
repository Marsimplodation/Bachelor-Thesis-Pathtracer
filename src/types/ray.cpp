#include "ray.h"
#include "common.h"
#include <algorithm>
#include <cmath>


// --- uniformly sample disk ---
Vector2 uniformSampleDisk(Ray & r) {
    using std::max;
    Vector2 sample;
    float theta = fastRandom(r.randomState);
    float sinTheta = sqrtf(theta);
    float phi = 2 * 3.14f * (fastRandom(r.randomState));
    sample.x = sinTheta * cos(phi);
    sample.y = sinTheta * sin(phi);
    return sample;
}

Vector3 randomCosineWeightedDirection(Ray & r) {
    using std::max;
    Vector3 direction;
    float theta = fastRandom(r.randomState);
    float sinTheta = sqrtf(theta);
    float phi = 2 * 3.14f * (fastRandom(r.randomState));
    direction.x = sinTheta * cos(phi);
    direction.y = sinTheta * sin(phi);
    direction.z = sqrt(max(0.0f, 1.0f - direction.x * direction.x - direction.y * direction.y));
    return direction;
}
Vector3 randomUniformDirection(Ray & r) {
    float z = 2.0f * fastRandom(r.randomState) - 1.0f; // Random z in range [-1, 1]
    float phi = 2.0f * 3.14f * fastRandom(r.randomState); // Random phi in range [0, 2π]
    
    float x = sqrt(1.0f - z * z) * cos(phi); // Convert to Cartesian coordinates
    float y = sqrt(1.0f - z * z) * sin(phi);

    return Vector3(x, y, z); // Return the uniformly distributed vector
}
