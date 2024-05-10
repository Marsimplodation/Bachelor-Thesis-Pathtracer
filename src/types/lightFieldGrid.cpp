#include "lightFieldGrid.h"
#include "common.h"
#include "scene/scene.h"
#include "types/aabb.h"
#include "types/vector.h"
#include <cmath>
#include <cstdio>
#include <Eigen>
#include <iostream>


namespace {
    Grid grid{};
    std::vector<int> indicies(0); 
    std::vector<int> gridLut(0); 
}

int getLUTIdx(int u, int v, int s, int t) {
    //??
    return 0;
    
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
    //to do get all tris in the lut for uvst and loop over them
}

bool triInUnitCube(Vector3* verts) {
    //need to come up with that
    return false;
}

void constructChannel(float u, float v, float s, float t) { 
    u = (u)/(float)grid.size.x;
    v = (v)/(float)grid.size.y;
    s = (s)/(float)grid.size.x;
    t = (t)/(float)grid.size.y;

    float deltaX = grid.max.x - grid.min.x;
    float deltaY = grid.max.y - grid.min.y;
    
    //the algorithm needs 4 points
    //choosen points:
    //front left bottom u,v
    //from right up u+1,v+1
    //back left up s,t+1
    //back right bottom s+1,t
    Vector3 points[] = {
        {grid.min.x + deltaX * u, grid.min.y + deltaY * v, grid.min.z},
        {grid.min.x + deltaX * (u+1.0f), grid.min.y + deltaY * (v+1.0f), grid.min.z},
        {grid.min.x + deltaX * s, grid.min.y + deltaY * (t+1.0f), grid.max.z},
        {grid.min.x + deltaX * (s+1.0f), grid.min.y + deltaY * (t), grid.max.z},
    };

    Eigen::Matrix<float, 12, 12> M0;
    M0.fill(0.0f);
    for (int i = 0; i < 4; ++i) {
        auto const p = points[i];
        for(int j = 0; j < 3; ++j) {
            M0(i*3+j, 0+j*4) = p.x;
            M0(i*3+j, 1+j*4) = p.y;
            M0(i*3+j, 2+j*4) = p.z;
            M0(i*3+j, 3+j*4) = 1.0f;
        }
    }

    //the points that are supposed to come out
    //0,0,0
    //1,1,0
    //0,1,1
    //1,0,1
    Eigen::Vector<float, 12> newPoints;
    newPoints <<0,0,0,
        1,1,0,
        0,1,1,
        1,0,1;

    Eigen::Vector<float, 12> M1Fields = M0.colPivHouseholderQr().solve(newPoints); 
    Eigen::Matrix<float, 4, 4> M1;
    M1 << M1Fields(0), M1Fields(1), M1Fields(2), M1Fields(3),
        M1Fields(4), M1Fields(5), M1Fields(6), M1Fields(7),
        M1Fields(8), M1Fields(9), M1Fields(10), M1Fields(11),
        0,0,0,1.0f;
   
    //transform each triangle in local space and test it against a unit cube
    Vector3 transformed[3];
    for(int i = 0; i < getObjectBuffer()->size(); ++i) {
        auto triangle = getObjectBuffer()->at(i);
        auto ov1 = triangle.vertices[0];
        auto ov2 = triangle.vertices[1];
        auto ov3 = triangle.vertices[2];
        Eigen::Vector<float, 4> v1;
        Eigen::Vector<float, 4> v2;
        Eigen::Vector<float, 4> v3;
        v1 << ov1.x, ov1.y, ov1.z, 1.0f;
        v2 << ov2.x, ov2.y, ov2.z, 1.0f;
        v3 << ov3.x, ov3.y, ov3.z, 1.0f;
        
        v1 = M1 * v1;
        v2 = M1 * v2;
        v3 = M1 * v3;

        transformed[0] = {v1(0), v1(1), v1(2)};
        transformed[1] = {v2(0), v2(1), v2(2)};
        transformed[2] = {v3(0), v3(1), v3(2)};
        if (!triInUnitCube(transformed)) continue;
        //indicies.push_back(i); 
    }
    //todo save indices in lut 
}

void printProgressBar(double progress, int barWidth = 70) {
    std::cout << "[";
    int pos = static_cast<int>(barWidth * progress);
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
}

void constructGrid() {
    grid.size = {10,10};
    float count = grid.size.x * grid.size.x * grid.size.y * grid.size.y;
    grid.min = getSceneMinBounds();
    grid.max = getSceneMaxBounds();
    printf("building channel LUT\n");
    int i = 1;
    for(int u = 0; u<grid.size.x; u++) {
        for(int v = 0; v<grid.size.y; v++) {
            for(int s = 0; s<grid.size.x; s++) {
                for(int t = 0; t<grid.size.y; t++) {
                    constructChannel(u, v, s, t);
                    printProgressBar(i++ / count);
                }
            }
        }
    }
    printf("\ndone building channel LUT\n");
}
