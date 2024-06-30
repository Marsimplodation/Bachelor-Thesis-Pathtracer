#include "primitive.h"
#include "scene/scene.h"
#include "shader/shader.h"
#include "types/vector.h"

//primitive compare
bool operator <(const PrimitiveCompare & p1, const PrimitiveCompare &p2) {
    return p1.val < p2.val;
}
