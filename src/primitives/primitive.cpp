#include "primitive.h"
#include "types/vector.h"

bool findIntersection(Ray &ray, void * primitive) {
    if(!primitive) return false;
    char flag = *((char*)primitive);
    switch(flag) {
        case CUBE:
            return findIntersection(ray, *(Cube*)primitive);
        case PLANE:
            return findIntersection(ray, *(Plane*)primitive);
        case TRIANGLE:
            return findIntersection(ray, *(Triangle*)primitive);
        case SPHERE:
            return findIntersection(ray, *(Sphere*)primitive);
        case OBJECT:
            return findIntersection(ray, *(Object*)primitive);
        default:
            return false;
    }
}


Vector3 minBounds(void * primitive) {
    if(!primitive) return {};
    char flag = *((char*)primitive);
    switch(flag) {
        case CUBE:
            return minBounds(*(Cube*)primitive);
        case PLANE:
            return minBounds(*(Plane*)primitive);
        case TRIANGLE:
            return minBounds(*(Triangle*)primitive);
        case SPHERE:
            return minBounds(*(Sphere*)primitive);
        case OBJECT:
            return minBounds(*(Object*)primitive);
        default:
            return {};
    }
}


Vector3 maxBounds(void * primitive) {
    if(!primitive) return {};
    char flag = *((char*)primitive);
    switch(flag) {
        case CUBE:
            return maxBounds(*(Cube*)primitive);
        case PLANE:
            return maxBounds(*(Plane*)primitive);
        case TRIANGLE:
            return maxBounds(*(Triangle*)primitive);
        case SPHERE:
            return maxBounds(*(Sphere*)primitive);
        case OBJECT:
            return maxBounds(*(Object*)primitive);
        default:
            return {};
    }
}


//primitive compare
bool operator <(const PrimitiveCompare & p1, const PrimitiveCompare &p2) {
    return p1.val < p2.val;
}
