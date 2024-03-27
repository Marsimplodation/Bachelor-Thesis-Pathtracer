#include "ray.h"
#include "common.h"
#include <cmath>


Vector3 randomV3UnitHemisphere(Ray & r) {
    Vector3 direction;
    float theta = fastRandom(r.randomState); //=sinf(acosf(cos));
    float sinTheta = sqrtf(1-theta*theta);
    float phi = 2 * 3.14f * (fastRandom(r.randomState));
    direction.x = sinTheta * cosf(phi);
    direction.y = sinTheta * sinf(phi);
    direction.z = theta;
    //adding the normal to attain the final random direction in the hemisphere
    if(dotProduct(r.normal, direction) < 0.0f) direction = -1*direction;
    direction = normalized(direction + r.normal);
  return direction;
}
