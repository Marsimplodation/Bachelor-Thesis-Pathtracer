#include "lightFieldGrid.h"
#include "common.h"
#include "scene/scene.h"
#include "src/Core/Matrix.h"
#include "types/aabb.h"
#include "types/vector.h"
#include <cmath>
#include <cstdio>
#include <Eigen>


namespace {
    Grid grid{};
    

}

void intersectGrid(Ray & r) {
    //first intersect
    Vector2 xRange = {grid.min.x, grid.max.x};
    Vector2 yRange = {grid.min.y, grid.max.y};
    float oz = r.origin.z - grid.min.z;
    float d = 0;
    if (fabs(oz) < EPS){}
    else d = oz / r.direction.z;
    float iX = r.origin.x + d*r.direction.x;
    float iY = r.origin.y + d*r.direction.y;
    
    //get uv coordinates
    float u = (iX - xRange.x)/xRange.y;
    if(u < 0.0f || u >= 1.0f) return;
    u = (int)(u * grid.size.x);
    float v = (iY - yRange.x)/yRange.y;
    if(v < 0.0f || v >= 1.0f) return;
    v = (int)(v * grid.size.y);

    //distance the two planes have
    //intersect with far plane
    float dGrid = grid.max.z - grid.min.z;
    oz+=dGrid;
    d = 0;
    if (fabs(oz) < EPS){}
    else d = oz / r.direction.z;
    iX = r.origin.x + d*r.direction.x;
    iY = r.origin.y + d*r.direction.y;

    //get st coordinates
    float s = (iX - xRange.x)/xRange.y;
    if(s < 0.0f || s >= 1.0f) return;
    s = (int)(s * grid.size.x);
    float t = (iY - yRange.x)/yRange.y;
    if(t < 0.0f || t >= 1.0f) return;
    t = (int)(t * grid.size.y);
    
    //ray is in channel uv,st
    //printf("u: %f, v: %f, s: %f, t: %f\n", u, v, s, t);
}

void constructChannel(float u, float v, float s, float t) { 
    u = (u+1.0f)/(float)grid.size.x;
    v = (v+1.0f)/(float)grid.size.y;
    s = (s+1.0f)/(float)grid.size.x;
    t = (t+1.0f)/(float)grid.size.y;
    

    Eigen::Matrix<float, 12, 12> m;



}

void constructGrid() {
    grid.size = {10,10};
    grid.min = getSceneMinBounds();
    printf("%f, %f, %f\n", grid.min.x, grid.min.y, grid.min.z);
    grid.max = getSceneMaxBounds(); 
    printf("%f, %f, %f\n", grid.max.x, grid.max.y, grid.max.z);
}
