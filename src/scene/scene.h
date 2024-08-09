#ifndef SCENE_H
#define SCENE_H
#include "../types/ray.h"
#include "../primitives/primitive.h"
#include "types/vector.h"
#include <cstdlib>
#include <vector>
#define KILLCHANCE 0.15
void findIntersection(Ray & r);
void findOcclusion(Ray & r);
void initScene();
void destroyScene();


std::vector<u32> & getIndicies();
std::vector<Triangle> & getTris();
std::vector<Object> & getObjects();

void findIntersection(Ray &ray);
Vector3 getSceneMinBounds();
Vector3 getSceneMaxBounds();

void resetScene();
void buildAS();

#endif // !SCENE_H
