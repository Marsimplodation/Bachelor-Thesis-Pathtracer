#ifndef LIGHT_FIELD_GRID_SUB_BEAMS_H
#define LIGHT_FIELD_GRID_SUB_BEAMS_H
#include "types/aabb.h"
#include "types/vector.h"
#include <vector>
#include "common.h"

struct GridSubBeams {
    int splitingAxis;
    Vector3 min;
    Vector3 max;
    AABB aabb;
    std::vector<u32> beams = std::vector<u32>();  
};

struct Beam {
    bool hasTris;
    Vector4 minUVST;
    Vector4 maxUVST;
    u32 startIdx;
    u32 endIdx;
    u32 parentIdx;
    std::vector<u32> indiciesForChilds;
};

 
u64 getMemoryGridBeams();
void setGridBeamsSettings(u32 size, u32 count);
void buildGridsWithSubBeams();
void intersectSubBeamGrid(Ray &r);
#endif // !LIGHT_FIELD_GRID_SUB_BEAMS_H
