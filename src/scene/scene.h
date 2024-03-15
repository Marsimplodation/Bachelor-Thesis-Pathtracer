#ifndef SCENE_H
#define SCENE_H
#include "../types/ray.h"
#define KILLCHANCE 0.05
void findIntersection(Ray & r);
void findOcclusion(Ray & r);
void initScene();

#endif // !SCENE_H
