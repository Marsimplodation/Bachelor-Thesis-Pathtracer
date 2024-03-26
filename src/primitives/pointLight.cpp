#include "pointLight.h"
#include "../scene/scene.h"
#include <cmath>
#include <cstdlib>


Vector3 illuminate(Ray & r, PointLight & light) {
    float radius = light.radius;
    float xi1 = ((fastRandom(r.randomState))*2-1.0f)*radius;
    float xi2 = ((fastRandom(r.randomState))*2 -1.0f)*radius;
    float xi3 = ((fastRandom(r.randomState))*2 -1.0f) *radius;
    Vector3 lightPos = {xi1, xi2, xi3};
    lightPos += light.center;

    Ray shadowRay = r;
    shadowRay.hit = false;
    shadowRay.origin = r.origin +r.direction * (r.length+0.01f);
    shadowRay.direction = normalized(lightPos - shadowRay.origin);
    shadowRay.length = length(lightPos - shadowRay.origin);
    findIntersection(shadowRay);
    float i = 1.0f/(shadowRay.length * shadowRay.length);
    if (i >= 1.0f) i=1.0f;
    i*= fabs(dotProduct(r.normal, shadowRay.direction));

    if(shadowRay.hit) i=0;
    return light.color * i;
}
