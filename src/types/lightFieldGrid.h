#ifndef LIGHT_FIELD_GRID_H
#define LIGHT_FIELD_GRID_H


#include "primitives/plane.h"
#include "types/vector.h"
#include <vector>
struct Grid {
    Plane uv, st;
    //experiment with others later
    std::vector<std::vector<int>> indices;  
};

struct GridSettings {
    Vector2 size;
};

#endif // !LIGHT_FIELD_GRID_H
