#ifndef LIGHT_FIELD_GRID_H
#define LIGHT_FIELD_GRID_H


#include "primitives/plane.h"
#include "types/bvh.h"
#include "types/vector.h"
#include <vector>

struct Grid {
    Vector3 min;
    Vector3 max;
    Vector2 size;
    //experiment with others later
    std::vector<std::vector<int>> indices; 
};
void intersectGrid(Ray & r);
void constructGrid();
#endif // !LIGHT_FIELD_GRID_H
