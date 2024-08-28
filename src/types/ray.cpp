#include "ray.h"
#include "common.h"
#include <algorithm>
#include <cmath>


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
