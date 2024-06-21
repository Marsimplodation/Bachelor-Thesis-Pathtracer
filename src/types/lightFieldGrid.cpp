#include "lightFieldGrid.h"
#include "common.h"
#include "primitives/triangle.h"
#include "primitives/primitive.h"
#include "scene/scene.h"
#include "types/aabb.h"
#include "types/camera.h"
#include "types/vector.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <Eigen>
#include <iostream>
#include <utility>


namespace {
    Grid grids[] = {
    {.splitingAxis = 0},
    {.splitingAxis = 1},
    {.splitingAxis = 2}
    };
    

    Vector2 getGridAxes(int idx) {
        switch (idx) {
            case 0:
                return {1,2};
            case 1:
                return {0,2};
            default:
                return {0,1};
        } 
    }
}

// Calculate the flattened index of the 4D LUT based on the dimensions and indices
int getLUTIdx(int u, int v, int s, int t, int idx) {
    return (u * grids[idx].size.x * grids[idx].size.y * grids[idx].size.x) + (v * grids[idx].size.x * grids[idx].size.y) + (s * grids[idx].size.x) + t;
}

void intersectGrid(Ray & r) {    
    float maxDelta = max(r.direction, true);
    int axis = (maxDelta == fabsf(r.direction[0])) ? 0 : (maxDelta == fabs(r.direction[1])) ? 1 : 2; 
    auto axes = getGridAxes(axis);
    if(fabsf(axes[0]) < fabsf(axes[1])) std::swap(axes[0], axes[1]);
    int idxs[] = {axis, (int)axes[0], (int)axes[1]};
    for (int it = 0; it < 3; ++it) {
        int idx = idxs[it];
        axis = idx;
    auto axes = getGridAxes(axis);
    int right = axes[0];
    int up = axes[1];

    //first intersect
    Vector2 rRange = {grids[idx].min[right], grids[idx].max[right]};
    Vector2 uRange = {grids[idx].min[up], grids[idx].max[up]};


    // First intersect
    float oz = grids[idx].min[axis] - r.origin[axis];
    float d = (fabs(oz) < EPS) ? 0 : oz / r.direction[axis];
    float iX = r.origin[right] + d * r.direction[right];
    float iY = r.origin[up] + d * r.direction[up];

    // Get uv coordinates
    float u = (iX - rRange.x) / (rRange.y - rRange.x);
    float v = (iY - uRange.x) / (uRange.y - uRange.x);

    // Check if uv coordinates are within range
    u = fmaxf(0.0f, fminf(u, 1.0f));
    v = fmaxf(0.0f, fminf(v, 1.0f));
    if(u < 0 || u > 1 || v < 0 || v > 1) {
        return;
    }

    // Convert to grid indices
    int uIndex = static_cast<int>(u * grids[idx].size.x);
    int vIndex = static_cast<int>(v * grids[idx].size.y);

    // Intersect with far plane
    oz = -(r.origin[axis] - grids[idx].max[axis]);
    d = (fabs(oz) < EPS) ? 0 : oz / r.direction[axis];
    iX = r.origin[right] + d * r.direction[right];
    iY = r.origin[up] + d * r.direction[up];

    // Get st coordinates
    float s = (iX - rRange.x) / (rRange.y - rRange.x);
    float t = (iY - uRange.x) / (uRange.y - uRange.x);
    s = fmaxf(0.0f, fminf(s, 1.0f));
    t = fmaxf(0.0f, fminf(t, 1.0f));


    // Check if st coordinates are within range
    if(s < 0 || s > 1 || t < 0 || t > 1)
        return;

    // Convert to grid indices
    int sIndex = static_cast<int>(s * grids[idx].size.x);
    int tIndex = static_cast<int>(t * grids[idx].size.y);
   
    //ray is in channel uv,st
    //to do get all tris in the lut for uvst and loop over them
    int lutIdx = getLUTIdx(uIndex, vIndex, sIndex, tIndex, idx);

    //sanity check
    if(lutIdx >= grids[idx].gridLutStart.size() || lutIdx >= grids[idx].gridLutEnd.size()) continue;
    int startIdx = grids[idx].gridLutStart.at(lutIdx);
    int endIdx = grids[idx].gridLutEnd.at(lutIdx);
    bool hit = false;
    for (unsigned int i = startIdx; i < endIdx; ++i) {
        if(r.terminated) break;
        //sanity check
        if(i<= 0 || i >= grids[idx].indicies.size()) break;
        int tIdx = grids[idx].indicies[i];
        r.interSectionTests++;
        Triangle & triangle = *getObjectBufferAtIdx(tIdx);
        hit |= triangleIntersection(r, triangle);
    }
    if(hit) return;
    }
}


void constructChannel(float u, float v, float s, float t, int idx) { 
    u = (u)/(float)grids[idx].size.x;
    v = (v)/(float)grids[idx].size.y;
    s = (s)/(float)grids[idx].size.x;
    t = (t)/(float)grids[idx].size.y;

    const auto axis = grids[idx].splitingAxis;
    const auto axes = getGridAxes(axis);
    const int right = axes[0];
    const int up = axes[1];

    float deltaX = grids[idx].max[right] - grids[idx].min[right];
    float deltaY = grids[idx].max[up] - grids[idx].min[up];
    
    //the algorithm needs 4 points
    //choosen points:
    //front left bottom u,v
    //from right up u+1,v+1
    //back left up s,t+1
    //back right bottom s+1,t
    Vector3 points[] = {
        {grids[idx].min.x + deltaX * u, grids[idx].min.y + deltaY * v, grids[idx].min.z},
        {grids[idx].min.x + deltaX * (u+1.0f), grids[idx].min.y + deltaY * (v+1.0f), grids[idx].min.z},
        {grids[idx].min.x + deltaX * s, grids[idx].min.y + deltaY * (t+1.0f), grids[idx].max.z},
        {grids[idx].min.x + deltaX * (s+1.0f), grids[idx].min.y + deltaY * (t), grids[idx].max.z},
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
    //-0.5,-0.5,-0.5
    //0.5,0.5,-0.5
    //-0.5,0.5,0.5
    //0.5,-0.5,0.5
    //this is because this is supposed to be a unit cube with the center being 0,0,0
    Eigen::Vector<float, 12> newPoints;
    newPoints <<-0.5f,-0.5f,-0.5f,
        0.5f,0.5f,-0.5f,
        -0.5f,0.5f,0.5f,
        0.5f,-0.5f,0.5f;

    Eigen::Vector<float, 12> M1Fields = M0.colPivHouseholderQr().solve(newPoints); 
    Eigen::Matrix<float, 4, 4> M1;
    M1 << M1Fields(0), M1Fields(1), M1Fields(2), M1Fields(3),
        M1Fields(4), M1Fields(5), M1Fields(6), M1Fields(7),
        M1Fields(8), M1Fields(9), M1Fields(10), M1Fields(11),
        0,0,0,1.0f;

    //transform each triangle in local space and test it against a unit cube
    int startIdx = grids[idx].indicies.size();
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
        grids[idx].indicies.push_back(i);
    }
    u = (u)*(float)grids[idx].size.x;
    v = (v)*(float)grids[idx].size.y;
    s = (s)*(float)grids[idx].size.x;
    t = (t)*(float)grids[idx].size.y;
    int lutIdx = getLUTIdx(u, v, s, t, idx);
    int endIdx = grids[idx].indicies.size();
    grids[idx].gridLutStart.at(lutIdx) = startIdx;
    grids[idx].gridLutEnd.at(lutIdx) = endIdx;
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


void adjustGridSize(int idx) {
    const auto axis = grids[idx].splitingAxis;
    const auto axes = getGridAxes(axis);
    const int right = axes[0];
    const int up = axes[1];

    //expand the grid just a tiny bit, to give wiggle room for floating errors during intersect testing
    grids[idx].min[axis] -= 0.5f;
    grids[idx].max[axis] += 0.5f;

    auto origin = getCamera()->origin;

    //move the camera origin so it is always outside of the grid, while maintaining distance
    if(origin[axis] >= grids[idx].min[axis]) origin[axis] -= 2 * (origin[axis] - grids[idx].min[axis]);

    float farDistance = grids[idx].max[axis] - origin[axis];
    
    //project the max point, which is on the far plane, on the near plane
    auto maxPoint = grids[idx].max; maxPoint[axis] = grids[idx].min[axis];
    auto dir = normalized(maxPoint - origin);

    //travel the direction untill reaching the farplane
    grids[idx].max[up] = fmaxf(grids[idx].max[up], origin[up] + dir[up] * (farDistance / dir[axis]));
    grids[idx].max[right] = fmaxf(grids[idx].max[right], origin[right] + dir[right] * (farDistance / dir[axis]));

    //extend the minPoint to the farPlane with the same method
    dir = normalized(grids[idx].min - origin);
    grids[idx].min[up] = fminf(grids[idx].min[up], origin[up] + dir[up] * (farDistance / dir[axis]));
    grids[idx].min[right] = fminf(grids[idx].min[right], origin[right] + dir[right] * (farDistance / dir[axis]));

    //expand the grid just a tiny bit, to give wiggle room for floating errors during intersect testing
    grids[idx].min[up] -= 0.5f;
    grids[idx].min[right] -= 0.5f;
    grids[idx].max[up] += 0.5f;
    grids[idx].max[right] += 0.5f;
}


void constructGrid() {
    for(int idx = 0; idx < 3; ++idx) {
        grids[idx].size = {10,10};
        float count = grids[idx].size.x * grids[idx].size.x * grids[idx].size.y * grids[idx].size.y;
        grids[idx].min = getSceneMinBounds();
        grids[idx].max = getSceneMaxBounds();
       
        //offset the min and max based on the camera
        adjustGridSize(idx); 

        grids[idx].indicies.clear();
        grids[idx].gridLutEnd.clear();
        grids[idx].gridLutEnd.resize(count);
        grids[idx].gridLutStart.clear();
        grids[idx].gridLutStart.resize(count);

        printf("building channel LUT for Grid%d\n", idx);
        int i = 1;
        for(int u = 0; u<grids[idx].size.x; u++) {
            for(int v = 0; v<grids[idx].size.y; v++) {
                for(int s = 0; s<grids[idx].size.x; s++) {
                    for(int t = 0; t<grids[idx].size.y; t++) {
                        constructChannel(u, v, s, t, idx);
                        printProgressBar(i++ / count);
                    }
                }
            }
        }
        printf("\ndone building channel LUT\n");
    }
}



//----------- Math stuff -----------//
bool triInUnitCube(Vector3* verts) {
    // Check if at least one vertex is inside the unit cube
    bool inCube = false;
    for (int i = 0; i < 3; i++) {
        float x = verts[i].x;
        float y = verts[i].y;
        float z = verts[i].z;
        if (x > 0.5 || y > 0.5 || z > 0.5) continue;
        if (x < -0.5 || y < -0.5 || z < -0.5) continue;
        inCube = true;
    }
    if (inCube) return true;

    // Unit cube spans from -0.5 to 0.5 in all axes
    const float cubeMin = -0.5f;
    const float cubeMax = 0.5f;

    // Helper function to project a point onto an axis
    auto projectPoint = [](const Vector3& p, const Vector3& axis) -> float {
        return p.x * axis.x + p.y * axis.y + p.z * axis.z;
    };

    // Check overlap on the coordinate axes
    for (int axis = 0; axis < 3; axis++) {
        float triMin = INFINITY, triMax = -INFINITY;
        float cubeMinProj = cubeMin, cubeMaxProj = cubeMax;

        for (int i = 0; i < 3; i++) {
            float proj;
            switch (axis) {
                case 0: proj = verts[i].x; break;
                case 1: proj = verts[i].y; break;
                case 2: proj = verts[i].z; break;
            }
            if (proj < triMin) triMin = proj;
            if (proj > triMax) triMax = proj;
        }

        if (triMax < cubeMinProj || triMin > cubeMaxProj) return false;
    }

    // Edge vectors of the triangle and cube
    Vector3 edges[3] = {
        {verts[1].x - verts[0].x, verts[1].y - verts[0].y, verts[1].z - verts[0].z},
        {verts[2].x - verts[1].x, verts[2].y - verts[1].y, verts[2].z - verts[1].z},
        {verts[0].x - verts[2].x, verts[0].y - verts[2].y, verts[0].z - verts[2].z}
    };
    Vector3 cubeVerts[8] = {
            {cubeMin, cubeMin, cubeMin}, {cubeMin, cubeMin, cubeMax},
            {cubeMin, cubeMax, cubeMin}, {cubeMin, cubeMax, cubeMax},
            {cubeMax, cubeMin, cubeMin}, {cubeMax, cubeMin, cubeMax},
            {cubeMax, cubeMax, cubeMin}, {cubeMax, cubeMax, cubeMax}
        };

    // Create the 9 axis on which the test is performed
    Vector3 testAxes[9];
    int idx = 0;
    for (int i = 0; i < 3; i++) {
        for (int axis = 0; axis < 3; axis++) {
            Vector3 coordAxis;
            switch (axis) {
                case 0: coordAxis = {1, 0, 0}; break;
                case 1: coordAxis = {0, 1, 0}; break;
                case 2: coordAxis = {0, 0, 1}; break;
            }
            testAxes[idx++] = crossProduct(edges[i], coordAxis);
        }
    }

    //perform the SAT test
    for (int i = 0; i < 9; i++) {
        float triMin = INFINITY, triMax = -INFINITY;
        float cubeMinProj = INFINITY, cubeMaxProj = -INFINITY;

        for (int j = 0; j < 3; j++) {
            float proj = projectPoint(verts[j], testAxes[i]);
            if (proj < triMin) triMin = proj;
            if (proj > triMax) triMax = proj;
        }

        for (int j = 0; j < 8; j++) {
            float proj = projectPoint(cubeVerts[j], testAxes[i]);
            if (proj < cubeMinProj) cubeMinProj = proj;
            if (proj > cubeMaxProj) cubeMaxProj = proj;
        }

        if (triMax < cubeMinProj || triMin > cubeMaxProj) return false;
    }

    return true;
}


