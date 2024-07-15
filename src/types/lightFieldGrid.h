#ifndef LIGHT_FIELD_GRID_H
#define LIGHT_FIELD_GRID_H


#include "types/bvh.h"
#include "types/vector.h"
#include <vector>
#include "common.h"
struct Grid {
    bool hasTris; 
    int splitingAxis;
    u32 AABBIDx;
    Vector3 min;
    Vector3 max;
    //experiment with others later
    std::vector<u32> indicies = std::vector<u32>(); 
    std::vector<u32> gridLutStart = std::vector<u32>(); 
    std::vector<u32> gridLutEnd = std::vector<u32>(); 
};
void intersectGrid(Ray & r);
void constructGrid();
void constructChannel(float u, float v, float s, float t, int idx,
                      bool isObject = false, Object *obj = 0x0);
#endif // !LIGHT_FIELD_GRID_H
