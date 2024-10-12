#include "types/aabb.h"
#include "types/vector.h"
#include <cmath>
#include <utility>
#include <algorithm>



bool findAABBIntersection(const Ray &r, const AABB &b) {
    // calculate intersection intervals
    float tx1 = (b.min.x - r.origin.x) * r.inv_dir.x;
    float tx2 = (b.max.x - r.origin.x) * r.inv_dir.x;
    float ty1 = (b.min.y - r.origin.y) * r.inv_dir.y;
    float ty2 = (b.max.y - r.origin.y) * r.inv_dir.y;
    float tz1 = (b.min.z - r.origin.z) * r.inv_dir.z;
    float tz2 = (b.max.z - r.origin.z) * r.inv_dir.z;
    // find min and max intersection t’s
    using std::max;
    using std::min;
    float tres[2] = {
        max(max(max(min(tx1, tx2), min(ty1, ty2)), min(tz1, tz2)), r.tmin),
        min(min(min(max(tx1, tx2), max(ty1, ty2)), max(tz1, tz2)), r.tmax)
    };
    // return result
    return tres[0] <= tres[1];
}

float getIntersectDistance(const Ray &r, const AABB &b) {
    // calculate intersection intervals
    float tx1 = (b.min.x - r.origin.x) * r.inv_dir.x;
    float tx2 = (b.max.x - r.origin.x) * r.inv_dir.x;
    float ty1 = (b.min.y - r.origin.y) * r.inv_dir.y;
    float ty2 = (b.max.y - r.origin.y) * r.inv_dir.y;
    float tz1 = (b.min.z - r.origin.z) * r.inv_dir.z;
    float tz2 = (b.max.z - r.origin.z) * r.inv_dir.z;
    // find min and max intersection t’s
    using std::max;
    using std::min;
    float tres[2] = {
        max(max(max(min(tx1, tx2), min(ty1, ty2)), min(tz1, tz2)), r.tmin),
        min(min(min(max(tx1, tx2), max(ty1, ty2)), max(tz1, tz2)), r.tmax)
    };
    // return result
    return tres[0] <= tres[1] ? tres[0] : INFINITY;
}
