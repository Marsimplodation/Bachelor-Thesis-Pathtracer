#ifndef LIGHT_FIELD_GRID_H
#define LIGHT_FIELD_GRID_H


#include "types/aabb.h"
#include "types/bvh.h"
#include "types/vector.h"
#include <cmath>
#include <vector>
#include "common.h"
struct Grid {
    bool hasTris; 
    int splitingAxis;
    float cost = INFINITY;
    u32 size;
    Vector3 min;
    Vector3 max;
    AABB aabb;
    //experiment with others later
    std::vector<u32> indicies = std::vector<u32>(); 
    std::vector<u32> gridLutStart = std::vector<u32>(); 
    std::vector<u32> gridLutEnd = std::vector<u32>(); 
};
void intersectGrid(Ray & r);
void constructGrid();
void constructChannel(float u, float v, float s, float t, int idx, std::vector<u32> indicies, bool isObject = false);
#endif // !LIGHT_FIELD_GRID_H
