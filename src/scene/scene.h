#ifndef SCENE_H
#define SCENE_H
#include "../types/ray.h"
#include "../primitives/triangle.h"
#include "types/vector.h"
#include <cstdlib>
#include <vector>
#define KILLCHANCE 0.7
void findIntersection(Ray & r);
void findOcclusion(Ray & r);
void initScene();
void destroyScene();

std::vector<Triangle> *getObjectBuffer();
Triangle* getObjectBufferAtIdx(int idx);
int getNumPrimitives();
void *getPrimitive(int idx);
void removePrimitive(int idx);
Vector3 getSceneMinBounds();
Vector3 getSceneMaxBounds();

void resetScene();
void buildAS(); 
#endif // !SCENE_H
