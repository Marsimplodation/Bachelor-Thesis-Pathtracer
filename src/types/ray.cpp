#include "ray.h"
#include "common.h"
#include <cmath>


Vector3 randomCosineWeightedDirection(Ray & r) {
    Vector3 direction;
    float theta = fastRandom(r.randomState); //=sinf(acosf(cos));
    float sinTheta = sqrtf(1-theta*theta);
    float phi = 2 * 3.14f * (fastRandom(r.randomState));
    direction.x = sinTheta * cosf(phi);
    direction.y = sinTheta * sinf(phi);
    direction.z = theta;
    direction = normalized(direction);
    if(dotProduct(r.normal, direction) < 0.0f) direction = -1*direction;
  return direction;
}
