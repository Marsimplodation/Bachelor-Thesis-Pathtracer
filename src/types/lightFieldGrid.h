#ifndef LIGHT_FIELD_GRID_H
#define LIGHT_FIELD_GRID_H


#include "types/aabb.h"
#include "types/bvh.h"
#include "types/vector.h"
#include <vector>
#include "common.h"
struct Grid {
    bool hasTris; 
    int splitingAxis;
    u32 size;
    Vector3 min;
    Vector3 max;
    Vector3 inv_delta;
    AABB aabb;
    //experiment with others later
    std::vector<u32> indicies = std::vector<u32>(); 
    std::vector<u32> gridLutStart = std::vector<u32>(); 
    std::vector<u32> gridLutEnd = std::vector<u32>(); 
};
void intersectGrid(Ray & r, int idx = -1);
void constructGrid();
void constructChannel(float u, float v, float s, float t, int idx, std::vector<u32> indicies, bool isObject = false);
void setGridSettings(u32 size, u32 oSize, u32 count);
unsigned long getMemory2Plane();
#endif // !LIGHT_FIELD_GRID_H
