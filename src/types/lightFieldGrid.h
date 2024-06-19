#ifndef LIGHT_FIELD_GRID_H
#define LIGHT_FIELD_GRID_H


#include "primitives/plane.h"
#include "types/bvh.h"
#include "types/vector.h"
#include <vector>

struct Grid {
    int splitingAxis;
    Vector3 min;
    Vector3 max;
    Vector2 size;
    //experiment with others later
    std::vector<int> indicies; 
    std::vector<int> gridLutStart; 
    std::vector<int> gridLutEnd; 
};
void intersectGrid(Ray & r);
void constructGrid();

//local functions
bool triInUnitCube(Vector3* verts);
#endif // !LIGHT_FIELD_GRID_H
