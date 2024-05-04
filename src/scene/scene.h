#ifndef SCENE_H
#define SCENE_H
#include "../types/ray.h"
#include "../primitives/triangle.h"
#include <cstdlib>
#include <vector>
#define KILLCHANCE 0.7
void findIntersection(Ray & r);
void findOcclusion(Ray & r);
void initScene();
void destroyScene();

std::vector<Triangle> *getObjectBuffer();

int getNumPrimitives();
void *getPrimitive(int idx);
void removePrimitive(int idx);

void resetScene();
#endif // !SCENE_H
