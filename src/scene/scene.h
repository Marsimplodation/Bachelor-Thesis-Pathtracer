#ifndef SCENE_H
#define SCENE_H
#include "../types/ray.h"
#define KILLCHANCE 0.10
void findIntersection(Ray & r);
void findOcclusion(Ray & r);
void initScene();
Vector3 getDirectLightSample(Ray & r);

#endif // !SCENE_H
