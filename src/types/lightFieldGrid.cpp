#include "lightFieldGrid.h"
#include "types/aabb.h"
#include "types/vector.h"

namespace {
    GridSettings settings{
    .size= {.x= 10, .y= 10}
    };
    Grid grid{};

}

bool inChannel(AABB & box, Vector3 * uvst) {
    Vector3 points[8];
    Vector3 directions[64];
    loadPoints(box, points);
    bool intersects = false;
    
    //construct the minkowski difference
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            directions[i * 8 + j] = points[i] - uvst[j];
        }
    }

    //check if the origin is included, by using the directions vectors
    //if <--d1--- o ---d2--> the dot product between d1 and d2 is negative
    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 64; ++j) {
            if(dotProduct(directions[i], directions[j]) < 0) {
                intersects = true;
                break;
            }
        }
        if(intersects) break;
    }

    return intersects;
}

void constructGrid() {
    
}
