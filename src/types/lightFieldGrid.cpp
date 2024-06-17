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
    Grid grid{};
    std::vector<int> indicies(0); 
    std::vector<int> gridLutStart(0); 
    std::vector<int> gridLutEnd(0); 
}

// Calculate the flattened index of the 4D LUT based on the dimensions and indices
int getLUTIdx(int u, int v, int s, int t) {
    return (u * grid.size.x * grid.size.y * grid.size.x) + (v * grid.size.x * grid.size.y) + (s * grid.size.x) + t;
}

void intersectGrid(Ray & r) {
    //first intersect
    Vector2 xRange = {grid.min.x, grid.max.x};
    Vector2 yRange = {grid.min.y, grid.max.y};

    // First intersect
    float oz = -(r.origin.z - grid.min.z);
    float d = (fabs(oz) < EPS) ? 0 : oz / r.direction.z;
    float iX = r.origin.x + d * r.direction.x;
    float iY = r.origin.y + d * r.direction.y;

    // Get uv coordinates
    float u = (iX - xRange.x) / (xRange.y - xRange.x);
    float v = (iY - yRange.x) / (yRange.y - yRange.x);

    // Check if uv coordinates are within range
    //u = fmaxf(0.0f, fminf(u, 1.0f));
    //v = fmaxf(0.0f, fminf(v, 1.0f));
    if(u < 0 || u > 1 || v < 0 || v > 1) {
        return;
    }

    // Convert to grid indices
    int uIndex = static_cast<int>(u * grid.size.x);
    int vIndex = static_cast<int>(v * grid.size.y);

    // Intersect with far plane
    oz = -(r.origin.z - grid.max.z);
    d = (fabs(oz) < EPS) ? 0 : oz / r.direction.z;
    iX = r.origin.x + d * r.direction.x;
    iY = r.origin.y + d * r.direction.y;

    // Get st coordinates
    float s = (iX - xRange.x) / (xRange.y - xRange.x);
    float t = (iY - yRange.x) / (yRange.y - yRange.x);
    //s = fmaxf(0.0f, fminf(s, 1.0f));
    //t = fmaxf(0.0f, fminf(t, 1.0f));


    // Check if st coordinates are within range
    if(s < 0 || s > 1 || t < 0 || t > 1)
        return;

    // Convert to grid indices
    int sIndex = static_cast<int>(s * grid.size.x);
    int tIndex = static_cast<int>(t * grid.size.y);
   
    //ray is in channel uv,st
    //to do get all tris in the lut for uvst and loop over them
    int lutIdx = getLUTIdx(uIndex, vIndex, sIndex, tIndex);
    int startIdx = gridLutStart.at(lutIdx);
    int endIdx = gridLutEnd.at(lutIdx);
    bool hit = false;
    for (int i = startIdx; i < endIdx; ++i) {
        if(r.terminated) break;;
        int idx = indicies.at(i);
        r.interSectionTests++;
        Triangle & triangle = *getObjectBufferAtIdx(idx);
        hit |= triangleIntersection(r, triangle);
        //if (hit) break;
    }
}


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
    int startIdx = indicies.size();
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
        indicies.push_back(i);
    }
    u = (u)*(float)grid.size.x;
    v = (v)*(float)grid.size.y;
    s = (s)*(float)grid.size.x;
    t = (t)*(float)grid.size.y;
    int lutIdx = getLUTIdx(u, v, s, t);
    int endIdx = indicies.size();
    gridLutStart.at(lutIdx) = startIdx;
    gridLutEnd.at(lutIdx) = endIdx;

    //reoder the indices based on distance
    Vector3 frontCenter = 
        {grid.min.x + deltaX * (u+0.5f), grid.min.y + deltaY * (v+0.5f), grid.min.z};
    
    auto comp = [frontCenter](int a, int b){
        auto A = getObjectBuffer()->at(a);
        auto B = getObjectBuffer()->at(b);
        return length(frontCenter - minBounds(A)) <  length(frontCenter - minBounds(B));
    };

    //std::sort(indicies.begin() + startIdx, indicies.begin() + endIdx,comp);
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


void adjustGridSize() {
    //expand the grid just a tiny bit, to give wiggle room for floating errors during intersect testing
    grid.min.z -= 0.5f;
    grid.max.z += 0.5f;

    auto origin = getCamera()->origin;
    float maxZ = grid.max.z - origin.z;
    
    //project the max point, which is on the far plane, on the near plane
    auto maxPoint = grid.max; maxPoint.z = grid.min.z;
    auto dir = normalized(maxPoint - origin);

    //travel the direction untill reaching the farplane
    grid.max.x = origin.x + dir.x * (maxZ / dir.z);
    grid.max.y = origin.y + dir.y * (maxZ / dir.z);

    //extend the minPoint to the farPlane with the same method
    dir = normalized(grid.min - origin);
    grid.min.x = origin.x + dir.x * (maxZ / dir.z);
    grid.min.y = origin.y + dir.y * (maxZ / dir.z);
}


void constructGrid() {
    grid.size = {10,10};
    float count = grid.size.x * grid.size.x * grid.size.y * grid.size.y;
    grid.min = getSceneMinBounds();
    grid.max = getSceneMaxBounds();
   
    //offset the min and max based on the camera
    adjustGridSize(); 

    gridLutEnd.resize(count);
    gridLutStart.resize(count);

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
