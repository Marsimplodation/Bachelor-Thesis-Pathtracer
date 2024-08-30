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
