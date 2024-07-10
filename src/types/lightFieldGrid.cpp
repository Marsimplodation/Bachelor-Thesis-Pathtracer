#include "lightFieldGrid.h"
#include "common.h"
#include "primitives/primitive.h"
#include "primitives/triangle.h"
#include "scene/scene.h"
#include "types/aabb.h"
#include "types/camera.h"
#include "types/vector.h"
#include <Eigen>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <sys/types.h>
#include <unordered_set>
#include <utility>

namespace {
Grid grids[] = {{.splitingAxis = 0}, {.splitingAxis = 1}, {.splitingAxis = 2}};
std::vector<Grid> objectGrids = std::vector<Grid>();
Vector2 getGridAxes(int idx) {
    switch (idx) {
    case 0:
        return {1, 2};
    case 1:
        return {0, 2};
    default:
        return {0, 1};
    }
}
#define SIZE 14

struct GridBound {
Vector3 min;
Vector3 max;
};
GridBound gridBounds[3];
} // namespace

// Calculate the flattened index of the 4D LUT based on the dimensions and indices
int getLUTIdx(int u, int v, int s, int t, int idx, bool isObject = false) {
    Grid &grid = isObject ? objectGrids[idx] : grids[idx];
    return (u * SIZE * SIZE * SIZE) +
           (v * SIZE * SIZE) + 
           (s * SIZE) + t;
}

void intersectObjectGrid(Ray &r, int idx, int uIndex, int vIndex, int sIndex, int tIndex) {
    auto & objectBuffer = getObjects();
    auto & indicieBuffer = getIndicies();
    auto & trisBuffer = getTris();
    auto &grid = objectGrids[idx];
    auto axis = grid.splitingAxis;
    auto axes = getGridAxes(axis);
    int right = axes[0];
    int up = axes[1];
    // ray is in channel uv,st
    // to do get all tris in the lut for uvst and loop over them
    int lutIdx = getLUTIdx(uIndex, vIndex, sIndex, tIndex, idx, true);

    // sanity check
    if (lutIdx >= grid.gridLutStart.size() || lutIdx >= grid.gridLutEnd.size())
        return;
    ;
    int startIdx = grid.gridLutStart.at(lutIdx);
    int endIdx = grid.gridLutEnd.at(lutIdx);
    bool hit = false;
    for (unsigned int i = startIdx; i < endIdx; ++i) {
        if (r.terminated)
            break;
        // sanity check
        if (i < 0 || i >= grid.indicies.size())
            break;
        int sIdx = grid.indicies[i];
        r.interSectionTests++;
        Triangle &triangle = trisBuffer[sIdx];
        hit |= triangleIntersection(r, triangle);
    }
}

void intersectGrid(Ray &r) {
    auto &objectBuffer = getObjects();
    auto &indicieBuffer = getIndicies();
    auto &trisBuffer = getTris();

    float maxDelta = max(r.direction, true);
    int axis = (maxDelta == fabsf(r.direction[0]))  ? 0
               : (maxDelta == fabs(r.direction[1])) ? 1
                                                    : 2;
    auto axes = getGridAxes(axis);
    if (fabsf(axes[0]) < fabsf(axes[1]))
        std::swap(axes[0], axes[1]);
    int idxs[] = {axis, (int)axes[0], (int)axes[1]};
    for (int it = 0; it < 3; ++it) {
        int idx = idxs[it];
        axis = idx;
        auto axes = getGridAxes(axis);
        int right = axes[0];
        int up = axes[1];
        auto b = gridBounds[axis];


        // First intersect
        float oz = b.min[axis] - r.origin[axis];
        float d = (fabs(oz) < EPS) ? 0 : oz / r.direction[axis];
        float iX = r.origin[right] + d * r.direction[right];
        float iY = r.origin[up] + d * r.direction[up];

        // Get uv coordinates
        float u = (iX - b.min[right]) / (b.max[right] - b.min[right]);
        float v = (iY - b.min[up]) / (b.max[up] - b.min[up]);

        // Check if uv coordinates are within range
        if(u >= 1) u = 0.99f;
        if(v >= 1) v = 0.99f;
        if(u < 0) u = 0;
        if(v < 0) v = 0;
        if (u < 0 || u > 1 || v < 0 || v > 1) {
            continue;
        }

        // Convert to grid indices
        int uIndex = static_cast<int>(u * SIZE);
        int vIndex = static_cast<int>(v * SIZE);

        // Intersect with far plane
        oz = -(r.origin[axis] - b.max[axis]);
        d = (fabs(oz) < EPS) ? 0 : oz / r.direction[axis];
        iX = r.origin[right] + d * r.direction[right];
        iY = r.origin[up] + d * r.direction[up];

        // Get st coordinates
        float s = (iX - b.min[right]) / (b.max[right] - b.min[right]);
        float t = (iY - b.min[up]) / (b.max[up] - b.min[up]);
        if(s >= 1) s = 0.99f;
        if(t >= 1) t = 0.99f;
        if(s < 0) s = 0;
        if(t < 0) t = 0;

        // Check if st coordinates are within range
        if (s < 0 || s > 1 || t < 0 || t > 1) {
            continue;
        }

        // Convert to grid indices
        int sIndex = static_cast<int>(s * SIZE);
        int tIndex = static_cast<int>(t * SIZE);


        // ray is in channel uv,st
        // to do get all tris in the lut for uvst and loop over them
        int lutIdx = getLUTIdx(uIndex, vIndex, sIndex, tIndex, idx);

        // sanity check
        if (lutIdx >= grids[idx].gridLutStart.size() ||
            lutIdx >= grids[idx].gridLutEnd.size()){
            continue;
        }
        int startIdx = grids[idx].gridLutStart.at(lutIdx);
        int endIdx = grids[idx].gridLutEnd.at(lutIdx);
        bool hit = false;
        for (unsigned int i = startIdx; i < endIdx; i += 3) {
            if (r.terminated)
                break;

            // sanity check
            if (i < 0 || i >= grids[idx].indicies.size())
                break;
            int tIdx = grids[idx].indicies[i] / 3.0f;

            r.interSectionTests++;
            Object &o = objectBuffer[tIdx];
            if (!findIntersection(r, o.boundingBox)) {
                continue;
            }

            intersectObjectGrid(r, tIdx * 3 + axis, uIndex, vIndex, sIndex, tIndex);
        }
        // if (hit)
        return;
    }
}

void constructChannel(float u, float v, float s, float t, int idx,
                      bool isObject = false, Object* obj = 0x0) {
    auto &objectBuffer = getObjects();
    auto &indicieBuffer = getIndicies();
    auto &trisBuffer = getTris();
    Grid &grid = isObject ? objectGrids[idx] : grids[idx];
    u = (u) / (float)SIZE;
    v = (v) / (float)SIZE;
    s = (s) / (float)SIZE;
    t = (t) / (float)SIZE;

    const auto axis = grid.splitingAxis;
    const auto axes = getGridAxes(axis);
    const int right = axes[0];
    const int up = axes[1];
    auto b = gridBounds[axis];

    float deltaR = b.max[right] - b.min[right];
    float deltaU = b.max[up] - b.min[up];
    // the algorithm needs 4 points
    // choosen points:
    // front left bottom u,v
    // from right up u+1,v+1
    // back left up s,t+1
    // back right bottom s+1,t
    Vector3 points[4] = {};
    points[0][axis] = b.min[axis];
    points[0][right] = b.min[right] + deltaR * u;
    points[0][up] = b.min[up] + deltaU * v;

    points[1][axis] = b.min[axis];
    points[1][right] = b.min[right] + deltaR * (u + 1.0f);
    points[1][up] = b.min[up] + deltaU * (v + 1.0f);
    
    points[2][axis] = b.max[axis];
    points[2][right] = b.min[right] + deltaR * s; 
    points[2][up] = b.min[up] + deltaU * (t + 1.0f);
    
    points[3][axis] = b.max[axis];
    points[3][right] = b.min[right] + deltaR * (s + 1.0f); 
    points[3][up] = b.min[up] + deltaU * (t);


    Eigen::Matrix<float, 12, 12> M0;
    M0.fill(0.0f);
    for (int i = 0; i < 4; ++i) {
        auto const p = points[i];
        for (int j = 0; j < 3; ++j) {
            M0(i * 3 + j, 0 + j * 4) = p.x;
            M0(i * 3 + j, 1 + j * 4) = p.y;
            M0(i * 3 + j, 2 + j * 4) = p.z;
            M0(i * 3 + j, 3 + j * 4) = 1.0f;
        }
    }

    // the points that are supposed to come out
    //-0.5,-0.5,-0.5
    // 0.5,0.5,-0.5
    //-0.5,0.5,0.5
    // 0.5,-0.5,0.5
    // this is because this is supposed to be a unit cube with the center being
    // 0,0,0
    points[0][axis]  = -0.5f;
    points[0][right]  = -0.5f;
    points[0][up]  = -0.5f;

    points[1][axis]  = -0.5f;
    points[1][right]  = 0.5f;
    points[1][up]  = 0.5f;
    
    points[2][axis]  = 0.5f;
    points[2][right]  = -0.5f;
    points[2][up]  = 0.5f;
    
    points[3][axis]  = 0.5f;
    points[3][right]  = 0.5f;
    points[3][up]  = -0.5f;
    Eigen::Vector<float, 12> newPoints;
    newPoints << points[0].x, points[0].y, points[0].z, 
                points[1].x, points[1].y, points[1].z,
                points[2].x, points[2].y, points[2].z,
                points[3].x, points[3].y, points[3].z;

    Eigen::Vector<float, 12> M1Fields =
        M0.colPivHouseholderQr().solve(newPoints);
    Eigen::Matrix<float, 4, 4> M1;
    M1 << M1Fields(0), M1Fields(1), M1Fields(2), M1Fields(3), M1Fields(4),
        M1Fields(5), M1Fields(6), M1Fields(7), M1Fields(8), M1Fields(9),
        M1Fields(10), M1Fields(11), 0, 0, 0, 1.0f;

    // transform each triangle in local space and test it against a unit cube
    int startIdx = grid.indicies.size();
    Vector3 transformed[3];

    if (!isObject) {
        for (int i = 0; i < objectBuffer.size() * 3; i += 3) {
            auto & primitive = objectBuffer[i/3];
            auto minP = minBounds(primitive);
            auto maxP = maxBounds(primitive);
            // Construct all 8 vertices of the cube
            Vector3 v[8];
            v[0] = minP;                        // (minX, minY, minZ)
            v[1] = {maxP[0], minP[1], minP[2]}; // (maxX, minY, minZ)
            v[2] = {minP[0], maxP[1], minP[2]}; // (minX, maxY, minZ)
            v[3] = {maxP[0], maxP[1], minP[2]}; // (maxX, maxY, minZ)
            
            v[4] = {minP[0], minP[1], maxP[2]}; // (minX, minY, maxZ)
            v[5] = {maxP[0], minP[1], maxP[2]}; // (maxX, minY, maxZ)
            v[6] = {minP[0], maxP[1], maxP[2]}; // (minX, maxY, maxZ)
            v[7] = {maxP[0], maxP[1], maxP[2]}; // (maxX, maxY, maxZ)

            Eigen::Vector<float, 4> vi;
            for (int i = 0; i < 8; ++i) {
                vi << v[i].x, v[i].y, v[i].z, 1.0f;
                vi = M1 * vi;
                v[i] = {vi(0), vi(1), vi(2)};
            }

            if (!CuboidInUnitCube(v)) {
                continue;
            }
            grid.indicies.push_back(i);
            grid.indicies.push_back(i + 1);
            grid.indicies.push_back(i + 2);
        }
    } else {
        if(!obj) return;
        for (int i = obj->startIdx; i < obj->endIdx; ++i) {
            auto & triangle = trisBuffer[i];
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
            if (!triInUnitCube(transformed)) {
                //continue;
            }
            grid.indicies.push_back(i);
        }
    }

    u = (u) * (float)SIZE;
    v = (v) * (float)SIZE;
    s = (s) * (float)SIZE;
    t = (t) * (float)SIZE;
    int lutIdx = getLUTIdx(u, v, s, t, idx, isObject);
    int endIdx = grid.indicies.size();
    grid.gridLutStart.at(lutIdx) = startIdx;
    grid.gridLutEnd.at(lutIdx) = endIdx;
}

void printProgressBar(double progress, int barWidth = 70) {
    std::cout << "[";
    int pos = static_cast<int>(barWidth * progress);
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos)
            std::cout << "=";
        else if (i == pos)
            std::cout << ">";
        else
            std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
}

void adjustGridSize(int idx) {
    auto & b = gridBounds[idx];
    // Use grid to access the Grid object
    const auto axis = idx;
    const auto axes = getGridAxes(axis);
    const int right = axes[0];
    const int up = axes[1];

    // expand the grid just a tiny bit, to give wiggle room for floating errors
    // during intersect testing
    const float offset = 0.5f;
    b.min[axis] -= offset;
    b.max[axis] += offset;
    float deltaF = b.max[axis] - b.min[axis];
    float deltaU = b.max[up] - b.min[up];
    float deltaR = b.max[right] - b.min[right];
    // expand the b just a tiny bit, to give wiggle room for floating errors
    // during intersect testing
    b.min[up] -= offset + deltaF;
    b.min[right] -= offset + deltaF;
    b.max[up] += offset + deltaF;
    b.max[right] += offset + deltaF;
}

void constructGrid() {
    auto &objectBuffer = getObjects();
    auto &indicieBuffer = getIndicies();
    auto &trisBuffer = getTris();
    

    for(int idx = 0; idx < 3; ++idx) {
        auto & b = gridBounds[idx];
        b.min = getSceneMinBounds();
        b.max = getSceneMaxBounds();
        adjustGridSize(idx);
    }
    float count = SIZE * SIZE * SIZE * SIZE;

    // object grid
    objectGrids.resize(objectBuffer.size() * 3);
    for (int idx = 0; idx < objectBuffer.size(); ++idx) {
        for (int axis = 0; axis < 3; ++axis) {
            auto & primitive = objectBuffer[idx];
            objectGrids[idx * 3 + axis].splitingAxis = axis;

            // offset the min and max based on the camera

            objectGrids[idx * 3 + axis].indicies.clear();
            objectGrids[idx * 3 + axis].gridLutEnd.clear();
            objectGrids[idx * 3 + axis].gridLutEnd.resize(count);
            objectGrids[idx * 3 + axis].gridLutStart.clear();
            objectGrids[idx * 3 + axis].gridLutStart.resize(count);

            printf("building channel LUT for %s Grid %d\n", primitive.name.c_str(), axis);
            int i = 1;
            for (int u = 0; u < SIZE; u++) {
                for (int v = 0; v < SIZE; v++) {
                    for (int s = 0; s < SIZE; s++) {
                        for (int t = 0; t < SIZE; t++) {
                            constructChannel(u, v, s, t, idx * 3 + axis, true, &primitive);
                            printProgressBar(i++ / count);
                        }
                    }
                }
            }
            printf("\n");
        }
    }

    for (int idx = 0; idx < 3; ++idx) {
        grids[idx].indicies.clear();
        grids[idx].gridLutEnd.clear();
        grids[idx].gridLutEnd.resize(count);
        grids[idx].gridLutStart.clear();
        grids[idx].gridLutStart.resize(count);

        printf("building channel LUT for Grid %d\n", idx);
        int i = 1;
        for (int u = 0; u < SIZE; u++) {
            for (int v = 0; v < SIZE; v++) {
                for (int s = 0; s < SIZE; s++) {
                    for (int t = 0; t < SIZE; t++) {
                        constructChannel(u, v, s, t, idx);
                        printProgressBar(i++ / count);
                    }
                }
            }
        }
        printf("\n");
    }
    printf("\ndone building channel LUT\n");
}

//----------- Math stuff -----------//
bool triInUnitCube(Vector3 *verts) {
    // Check if at least one vertex is inside the unit cube
    // Unit cube spans from -0.5 to 0.5 in all axes
    const float cubeMin = -0.52f;
    const float cubeMax = 0.52f;

    // Helper function to project a point onto an axis
    auto projectPoint = [](const Vector3 &p, const Vector3 &axis) -> float {
        return p.x * axis.x + p.y * axis.y + p.z * axis.z;
    };

    // Check overlap on the coordinate axes
    for (int axis = 0; axis < 3; axis++) {
        float triMin = INFINITY, triMax = -INFINITY;
        float cubeMinProj = cubeMin, cubeMaxProj = cubeMax;

        for (int i = 0; i < 3; i++) {
            float proj;
            switch (axis) {
            case 0:
                proj = verts[i].x;
                break;
            case 1:
                proj = verts[i].y;
                break;
            case 2:
                proj = verts[i].z;
                break;
            }
            if (proj < triMin)
                triMin = proj;
            if (proj > triMax)
                triMax = proj;
        }

        if (triMax < cubeMinProj || triMin > cubeMaxProj)
            return false;
    }

    // Edge vectors of the triangle and cube
    Vector3 edges[3] = {{verts[1].x - verts[0].x, verts[1].y - verts[0].y,
                         verts[1].z - verts[0].z},
                        {verts[2].x - verts[1].x, verts[2].y - verts[1].y,
                         verts[2].z - verts[1].z},
                        {verts[0].x - verts[2].x, verts[0].y - verts[2].y,
                         verts[0].z - verts[2].z}};
    Vector3 cubeVerts[8] = {
        {cubeMin, cubeMin, cubeMin}, {cubeMin, cubeMin, cubeMax},
        {cubeMin, cubeMax, cubeMin}, {cubeMin, cubeMax, cubeMax},
        {cubeMax, cubeMin, cubeMin}, {cubeMax, cubeMin, cubeMax},
        {cubeMax, cubeMax, cubeMin}, {cubeMax, cubeMax, cubeMax}};

    // Create the 9 axis on which the test is performed
    Vector3 testAxes[9];
    int idx = 0;
    for (int i = 0; i < 3; i++) {
        for (int axis = 0; axis < 3; axis++) {
            Vector3 coordAxis;
            switch (axis) {
            case 0:
                coordAxis = {1, 0, 0};
                break;
            case 1:
                coordAxis = {0, 1, 0};
                break;
            case 2:
                coordAxis = {0, 0, 1};
                break;
            }
            testAxes[idx++] = crossProduct(edges[i], coordAxis);
        }
    }

    // perform the SAT test
    for (int i = 0; i < 9; i++) {
        float triMin = INFINITY, triMax = -INFINITY;
        float cubeMinProj = INFINITY, cubeMaxProj = -INFINITY;

        for (int j = 0; j < 3; j++) {
            float proj = projectPoint(verts[j], testAxes[i]);
            if (proj < triMin)
                triMin = proj;
            if (proj > triMax)
                triMax = proj;
        }

        for (int j = 0; j < 8; j++) {
            float proj = projectPoint(cubeVerts[j], testAxes[i]);
            if (proj < cubeMinProj)
                cubeMinProj = proj;
            if (proj > cubeMaxProj)
                cubeMaxProj = proj;
        }

        if (triMax < cubeMinProj || triMin > cubeMaxProj)
            return false;
    }

    return true;
}

bool CuboidInUnitCube(Vector3 *verts) {
    // Unit cube spans from -0.5 to 0.5 in all axes
    const float cubeMin = -0.52f;
    const float cubeMax = 0.52f;

    // Helper function to project a point onto an axis
    auto projectPoint = [](const Vector3 &p, const Vector3 &axis) -> float {
        return p.x * axis.x + p.y * axis.y + p.z * axis.z;
    };

    // Check overlap on the coordinate axes
    for (int axis = 0; axis < 3; axis++) {
        float triMin = INFINITY, triMax = -INFINITY;
        float cubeMinProj = cubeMin, cubeMaxProj = cubeMax;

        for (int i = 0; i < 8; i++) {
            float proj;
            switch (axis) {
            case 0:
                proj = verts[i].x;
                break;
            case 1:
                proj = verts[i].y;
                break;
            case 2:
                proj = verts[i].z;
                break;
            }
            if (proj < triMin)
                triMin = proj;
            if (proj > triMax)
                triMax = proj;
        }

        if (triMax < cubeMinProj || triMin > cubeMaxProj)
            return false;
    }

    // Edges of the AABB (only need 3 edges since they are axis-aligned)
    Vector3 edgesA[3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

    // Edges of the cuboid
    Vector3 edgesB[12] = {
        verts[1] - verts[0],
        verts[2] - verts[0],
        verts[4] - verts[0],
        verts[3] - verts[1],
        verts[5] - verts[1],
        verts[3] - verts[2],
        verts[6] - verts[2],
        verts[6] - verts[4],
        verts[5] - verts[4],
        verts[7] - verts[5],
        verts[7] - verts[3],
        verts[7] - verts[6],
    };

    // Create test axes from cross products of edges
    Vector3 testAxes[36];
    int idx = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 12; j++) {
            testAxes[idx++] = crossProduct(edgesA[i], edgesB[j]);
        }
    }

    Vector3 cubeVerts[8] = {
        {cubeMin, cubeMin, cubeMin}, {cubeMin, cubeMin, cubeMax},
        {cubeMin, cubeMax, cubeMin}, {cubeMin, cubeMax, cubeMax},
        {cubeMax, cubeMin, cubeMin}, {cubeMax, cubeMin, cubeMax},
        {cubeMax, cubeMax, cubeMin}, {cubeMax, cubeMax, cubeMax}};
    // perform the SAT test
    for (int i = 0; i < 15; i++) {
        float triMin = INFINITY, triMax = -INFINITY;
        float cubeMinProj = INFINITY, cubeMaxProj = -INFINITY;

        for (int j = 0; j < 8; j++) {
            float proj = projectPoint(verts[j], testAxes[i]);
            if (proj < triMin)
                triMin = proj;
            if (proj > triMax)
                triMax = proj;
        }

        for (int j = 0; j < 8; j++) {
            float proj = projectPoint(cubeVerts[j], testAxes[i]);
            if (proj < cubeMinProj)
                cubeMinProj = proj;
            if (proj > cubeMaxProj)
                cubeMaxProj = proj;
        }

        if (triMax < cubeMinProj || triMin > cubeMaxProj)
            return false;
    }

    return true;
}
