#include "ray.h"
#include "common.h"
#include <cmath>


Vector3 randomCosineWeightedDirection(Ray & r) {
    Vector3 direction;
    float theta = fastRandom(r.randomState); //=sinf(acosf(cos));
    float sinTheta = sqrtf(theta);
    float phi = 2 * 3.14f * (fastRandom(r.randomState));
    direction.x = sinTheta * cosf(phi);
    direction.y = sinTheta * sinf(phi);
    direction.z = sqrtf(fmaxf(0.0f, 1.0f - direction.x * direction.x - direction.y * direction.y));
    normalize(direction);
    if(dotProduct(r.normal, direction) < 0.0f) direction = -1*direction;
    return direction;
}
