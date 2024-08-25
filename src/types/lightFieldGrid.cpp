#include "lightFieldGrid.h"
#include "common.h"
#include "primitives/object.h"
#include "primitives/primitive.h"
#include "primitives/triangle.h"
#include "scene/scene.h"
#include "types/aabb.h"
#include "types/bvh.h"
#include "types/camera.h"
#include "types/sat.h"
#include "types/vector.h"
#include <Eigen>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <execution>
#include <iostream>
#include <sys/types.h>
#include <unordered_set>
#include <utility>

namespace {
Grid grids[] = {{.splitingAxis = 0}, {.splitingAxis = 1}, {.splitingAxis = 2}};
std::vector<Grid> objectGrids = std::vector<Grid>();

//------- helper functions ----//
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
int TRIS_GRID_SIZE = 4;
int GRID_SIZE = 8;
int MAX_TRIS_IN_CHANNEL = 50;
// Calculate the flattened index of the 4D LUT based on the dimensions and
// indices
inline int getLUTIdx(float u, float v, float s, float t, int idx, bool isObject = false) {
    Grid &grid = isObject ? objectGrids[idx] : grids[idx];
    u = (int) (u*grid.size);
    v = (int) (v*grid.size);
    s = (int) (s*grid.size);
    t = (int) (t*grid.size);
    return (u * grid.size * grid.size * grid.size) + (v * grid.size * grid.size) + (s * grid.size) + t;
}
thread_local std::vector<u32> toTraverse(0);

} // namespace

//settings
void setGridSettings(u32 size, u32 oSize, u32 count) {
    TRIS_GRID_SIZE = oSize;
    GRID_SIZE = size;
    MAX_TRIS_IN_CHANNEL = count;
}

unsigned long getMemory2Plane() {
    unsigned long size = (objectGrids.size() + 3) * sizeof(Grid);
    for(auto & g: objectGrids) {
        size += g.gridLutEnd.size() * sizeof(u32);
        size += g.gridLutStart.size() * sizeof(u32);
        size += g.indicies.size() * sizeof(u32);
    }
    for(auto & g: grids) {
        size += g.gridLutEnd.size() * sizeof(u32);
        size += g.gridLutStart.size() * sizeof(u32);
        size += g.indicies.size() * sizeof(u32);
    }
    return size;
}


void intersectGrid(Ray &r) {
    auto &objectBuffer = getObjects();
    auto &indicieBuffer = getIndicies();
    auto &trisBuffer = getTris();

    float maxDelta = 0.0f;
    int axis = 0;
    float f0 = std::abs(r.direction[0]);
    float f1 = std::abs(r.direction[1]);
    float f2 = std::abs(r.direction[2]);

    if (f0 > maxDelta) {
        maxDelta = f0;
        axis = 0;
    }
    if (f1 > maxDelta) {
        maxDelta = f1;
        axis = 1;
    }
    if (f2 > maxDelta) {
        maxDelta = f2;
        axis = 2;
    }
    
    int idx = axis;
    auto axes = getGridAxes(axis);
    int right = axes[0];
    int up = axes[1];

    bool topLevel = true;
    toTraverse.clear();
    toTraverse.push_back(idx);
    Vector4 points{};
    while(toTraverse.size() > 0) {
        idx = toTraverse.back();
        toTraverse.pop_back();
        auto & grid = topLevel ? grids[idx] : objectGrids[idx];
        
        //perform grid interesection
        r.interSectionAS++;
        float d1 = (grid.min[axis] - r.origin[axis]) * r.inv_dir[axis];
        float d2 = (grid.max[axis] - r.origin[axis]) * r.inv_dir[axis];
        bool gridMiss = std::max(d1, d2) < 0;
        if (gridMiss) continue; 

        r.interSectionAS++;
        if(!findIntersection(r, grid.aabb)) continue;
       
        //get intersection point
        auto in = (r.origin + d1 * r.direction - grid.min) * grid.inv_delta;
        auto out = (r.origin + d2 * r.direction - grid.min) * grid.inv_delta;

        gridMiss |= (in[right] < 0 || in[right] >= 1); 
        gridMiss |= (in[up] < 0 || in[up] >= 1); 
        gridMiss |= (out[right] < 0 || out[right] >= 1); 
        gridMiss |= (out[up] < 0 || out[up] >= 1); 
        if (gridMiss) continue; 


        // ray is in channel uv,st
        // to do get all tris in the lut for uvst and loop over them
        float lutIdx = getLUTIdx(in[right], in[up], out[right], out[up], idx, !topLevel);
        
        u32 startIdx = grid.gridLutStart[(lutIdx)];
        u32 endIdx = grid.gridLutEnd[(lutIdx)];
        if(endIdx - startIdx == 0) continue;
        if(grid.hasTris) {
            for (unsigned int i = startIdx; i < endIdx; ++i) {
                u32 sIdx = grid.indicies[i];
                r.interSectionTests++;
                Triangle &triangle = trisBuffer[sIdx];
                triangleIntersection(r, triangle);
            }
        } else if (r.direction[axis] < 0) {
            //ensure the closest aabb is traversed first (this is a stack, so it has to be last in the loop)
            for (unsigned int i = startIdx; i < endIdx; ++i) {
                u32 sIdx = grid.indicies[i];
                if(objectGrids[sIdx].aabb.min[axis] > r.origin[axis]) continue;
                toTraverse.push_back(sIdx);
            }
        } else {
            //ensure the closest aabb is traversed first (this is a stack, so it has to be last in the loop)
            for (unsigned int i = endIdx - 1; i != startIdx -1; --i) {
                u32 sIdx = grid.indicies[i];
                if(objectGrids[sIdx].aabb.max[axis] < r.origin[axis]) continue;
                toTraverse.push_back(sIdx);
            }
            
        }
        topLevel = false;
    }
}

//------------- Constructing the Grids ------------------//
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

void adjustGridSize(int idx, bool isObject = false) {
    Grid &grid = isObject ? objectGrids[idx] : grids[idx];
    // Use grid to access the Grid object
    const auto axis = grid.splitingAxis;
    const auto axes = getGridAxes(axis);
    const int right = axes[0];
    const int up = axes[1];

    // expand the grid just a tiny bit, to give wiggle room for floating errors
    // during intersect testing
    const float offset = 0.1f;
    grid.min[axis] -= offset;
    grid.max[axis] += offset;
    float deltaF = grid.max[axis] - grid.min[axis];
    float deltaU = grid.max[up] - grid.min[up];
    float deltaR = grid.max[right] - grid.min[right];
    // expand the b just a tiny bit, to give wiggle room for floating errors
    // during intersect testing
    grid.min[up] -= offset + deltaF;
    grid.min[right] -= offset + deltaF;
    grid.max[up] += offset + deltaF;
    grid.max[right] += offset + deltaF;
}

void constructGrid(int gridIdx) {
    auto &trisBuffer = getTris();
    
    std::vector<u32> gridsToBuild(0);
    gridsToBuild.push_back(gridIdx);
    const u32 originalNodeIdx = gridIdx;
            
    std::vector<u32> bvhIndicies(0);
    std::vector<u32> childs1(0);
    std::vector<u32> childs2(0);
    std::vector<u32> childs3(0);

    for(int n = 0; n < gridsToBuild.size(); ++n) {
        gridIdx = gridsToBuild[n];
    
        Grid & grid = objectGrids[gridIdx];
        BvhNode & node = getNode(grid.indicies.back());
        grid.aabb = getNodeAABB(node.AABBIdx);
        grid.min = grid.aabb.min;
        grid.max = grid.aabb.max;
        grid.indicies.pop_back();
        bvhIndicies.clear();
        childs1.clear();
        childs2.clear();
        childs3.clear();
        
        //set defaults
        
        if(grid.indicies.size() > MAX_TRIS_IN_CHANNEL && !isLeaf(node)) {
            childs1.push_back(node.childLeft);
            childs1.push_back(node.childRight);
            for(auto nIdx : childs1) {
                auto & n = getNode(nIdx);
                if(isLeaf(n)) childs2.push_back(nIdx);
                else {
                    childs2.push_back(n.childLeft);
                    childs2.push_back(n.childRight);
                }
            }
            
            for(auto nIdx : childs2) {
                auto & n = getNode(nIdx);
                if(isLeaf(n)) childs3.push_back(nIdx);
                else {
                    childs3.push_back(n.childLeft);
                    childs3.push_back(n.childRight);
                }

            }
            for(auto nIdx : childs3) {
                auto & n = getNode(nIdx);
                if(isLeaf(n)) bvhIndicies.push_back(nIdx);
                else {
                    bvhIndicies.push_back(n.childLeft);
                    bvhIndicies.push_back(n.childRight);
                }
            }
            
            grid.indicies.clear();
            grid.hasTris = false;
            grid.size = GRID_SIZE;
            int splitAxis=grid.splitingAxis;


            for(auto nIdx : bvhIndicies) {
                auto & n = getNode(nIdx);
                objectGrids.push_back(Grid{.splitingAxis = splitAxis});
                u32 size = objectGrids.size();
                auto & grid = objectGrids[gridIdx];
                grid.indicies.push_back(size - 1);
                
                auto & child = objectGrids[size - 1];
                child.indicies.clear();
                for(int i = n.startIdx; i < n.endIdx; ++i) {
                    child.indicies.push_back(bvhGetTrisIndex(i));
                }
                
                child.indicies.push_back(nIdx);
                child.aabb = getNodeAABB(n.AABBIdx);
                gridsToBuild.push_back(size-1);
            }
        } else {
            Grid & grid = objectGrids[gridIdx];
            grid.hasTris = true;
            int tris = grid.indicies.size();
            grid.size = TRIS_GRID_SIZE; 
        }
        Grid & newGrid = objectGrids[gridIdx];
        float count = newGrid.size * newGrid.size * newGrid.size * newGrid.size;
        newGrid.gridLutEnd.clear();
        newGrid.gridLutEnd.resize(count);
        newGrid.gridLutStart.clear();
        newGrid.gridLutStart.resize(count);
        adjustGridSize(gridIdx, true);
        newGrid.inv_delta[0] = 1.0f / (newGrid.max[0] - newGrid.min[0]);
        newGrid.inv_delta[1] = 1.0f / (newGrid.max[1] - newGrid.min[1]);
        newGrid.inv_delta[2] = 1.0f / (newGrid.max[2] - newGrid.min[2]);
        
        std::vector<u32> indicies = std::vector<u32>();
        for(auto idx : newGrid.indicies) {
            indicies.push_back(idx);
        }
        newGrid.indicies.clear();
        int i = 1;
        for (int u = 0; u < newGrid.size; u++) {
            for (int v = 0; v < newGrid.size; v++) {
                for (int s = 0; s < newGrid.size; s++) {
                    for (int t = 0; t < newGrid.size; t++) {
                        constructChannel(u, v, s, t, gridIdx, indicies, true);
                    }
                }
            }
        }
    }
}

void constructGrid() {
    auto &objectBuffer = getObjects();
    auto &indicieBuffer = getIndicies();
    auto &trisBuffer = getTris();
    std::vector<u32> indicies[] = {std::vector<u32>(0), std::vector<u32>(0), std::vector<u32>(0)};

    // object grid
    objectGrids.resize(0);
    for (int idx = 0; idx < objectBuffer.size(); ++idx) {
        for (int axis = 0; axis < 3; ++axis) {
            auto &primitive = objectBuffer[idx];
            auto grid = Grid{.splitingAxis = axis};
            grid.aabb = {.min = minBounds(primitive), .max=maxBounds(primitive)}; 
            grid.indicies = std::vector<u32>();
            grid.size = GRID_SIZE;
            for (int i = primitive.startIdx; i < primitive.endIdx; ++i) {
                grid.indicies.push_back(i);
            }
            grid.indicies.push_back(primitive.root);
            objectGrids.push_back(grid);
            indicies[axis].push_back(objectGrids.size() - 1);
            constructGrid(objectGrids.size() - 1);
            printf("%s\n", primitive.name.c_str());
        }
    }

    for (int idx = 0; idx < 3; ++idx) {
        grids[idx].size = GRID_SIZE;
        float count = grids[idx].size * grids[idx].size * grids[idx].size * grids[idx].size;
        grids[idx].indicies.clear();
        grids[idx].gridLutEnd.clear();
        grids[idx].gridLutEnd.resize(count);
        grids[idx].gridLutStart.clear();
        grids[idx].gridLutStart.resize(count);
        grids[idx].min = getSceneMinBounds();
        grids[idx].max = getSceneMaxBounds();
        grids[idx].aabb = {.min = grids[idx].min, .max = grids[idx].max};

        // offset the min and max based on the camera
        adjustGridSize(idx);
        grids[idx].inv_delta[0] = 1.0f / (grids[idx].max[0] - grids[idx].min[0]);
        grids[idx].inv_delta[1] = 1.0f / (grids[idx].max[1] - grids[idx].min[1]);
        grids[idx].inv_delta[2] = 1.0f / (grids[idx].max[2] - grids[idx].min[2]);
        grids[idx].hasTris = false;
        
        printf("building channel LUT for Grid %d\n", idx);
        int i = 1;
        for (int u = 0; u < grids[idx].size; u++) {
            for (int v = 0; v < grids[idx].size; v++) {
                for (int s = 0; s < grids[idx].size; s++) {
                    for (int t = 0; t < grids[idx].size; t++) {
                        constructChannel(u, v, s, t, idx, indicies[grids[idx].splitingAxis]);
                        printProgressBar(i++ / count);
                    }
                }
            }
        }
        printf("\n");
    }
    printf("\ndone building channel LUT\n");
}


//------------------ Channel construction ---------------//
void testChannelAgainstAABB(Grid &grid, int axis, int up, int right,
                            Vector3 *points, Vector3 *edges,std::vector<u32> & indicies) {
    auto &objectBuffer = getObjects();
    auto &indicieBuffer = getIndicies();    
    auto &trisBuffer = getTris();
    struct Compare {u32 idx; float d;};
    std::vector<Compare> compare(0);
    compare.reserve(indicies.size());

    for (auto i : indicies) {
        if(objectGrids[i].splitingAxis != grid.splitingAxis) continue;
        if (!cuboidInAABB(objectGrids[i].aabb, points, edges)) {
            continue;
        }
        compare.push_back({i, objectGrids[i].aabb.min[axis]});
    }
    std::sort(compare.begin(), compare.end(), [](Compare &a, Compare &b){return a.d < b.d;});
    for(auto c : compare) {
        grid.indicies.push_back(c.idx);
    }
}

void testChannelAgainstTriangles(Grid &grid, int axis, int up,
                                 int right, Vector3 *points, Vector3 *edges, std::vector<u32> & indicies) {
    auto &objectBuffer = getObjects();
    auto &indicieBuffer = getIndicies();
    auto &trisBuffer = getTris();

    for (auto i : indicies) {
        if (!triInChannel(trisBuffer[i].vertices, points, edges)) {
            continue;
        }
        grid.indicies.push_back(i);
    }

    /* old system solving the same problem with matrices
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
    points[0][axis] = -0.5f;
    points[0][right] = -0.5f;
    points[0][up] = -0.5f;

    points[1][axis] = -0.5f;
    points[1][right] = 0.5f;
    points[1][up] = 0.5f;

    points[2][axis] = 0.5f;
    points[2][right] = -0.5f;
    points[2][up] = 0.5f;

    points[3][axis] = 0.5f;
    points[3][right] = 0.5f;
    points[3][up] = -0.5f;
    AABB unitCUbe = {.min = {-0.5001f, -0.5001f, -0.5001f}, .max = {0.5001f, 0.5001f, 0.5001f}};

    Eigen::Vector<float, 12> newPoints;
    newPoints << points[0].x, points[0].y, points[0].z, points[1].x,
        points[1].y, points[1].z, points[2].x, points[2].y, points[2].z,
        points[3].x, points[3].y, points[3].z;

    Eigen::Vector<float, 12> M1Fields =
        M0.colPivHouseholderQr().solve(newPoints);
    Eigen::Matrix<float, 4, 4> M1;
    M1 << M1Fields(0), M1Fields(1), M1Fields(2), M1Fields(3), M1Fields(4),
        M1Fields(5), M1Fields(6), M1Fields(7), M1Fields(8), M1Fields(9),
        M1Fields(10), M1Fields(11), 0, 0, 0, 1.0f;

    Vector3 transformed[3];

    for (int i : indicies) {
        auto &triangle = trisBuffer[i];
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
        if (!triInAABB(unitCUbe, transformed)) {
            continue;
        }
        grid.indicies.push_back(i);
    }*/
}

void constructChannel(float u, float v, float s, float t, int idx, std::vector<u32> indicies, bool isObject) {
    auto &objectBuffer = getObjects();
    auto &indicieBuffer = getIndicies();
    auto &trisBuffer = getTris();
    Grid &grid = isObject ? objectGrids[idx] : grids[idx];
    u = (u) / (float)grid.size;
    v = (v) / (float)grid.size;
    s = (s) / (float)grid.size;
    t = (t) / (float)grid.size;

    const auto axis = grid.splitingAxis;
    const auto axes = getGridAxes(axis);
    const int right = axes[0];
    const int up = axes[1];

    float deltaR = grid.max[right] - grid.min[right];
    float deltaU = grid.max[up] - grid.min[up];
    // the algorithm needs 4 points
    // choosen points:
    // front left bottom u,v
    // from right up u+1,v+1
    // back left up s,t+1
    // back right bottom s+1,t
    // the rest of the 4 points are used when comparing with the object aabb
    Vector3 points[8] = {};
    points[0][axis] = grid.min[axis];
    points[0][right] = grid.min[right] + deltaR * u;
    points[0][up] = grid.min[up] + deltaU * v;

    points[1][axis] = grid.min[axis];
    points[1][right] = grid.min[right] + deltaR * (u + 1.0f/grid.size);
    points[1][up] = grid.min[up] + deltaU * (v + 1.0f/grid.size);

    points[2][axis] = grid.max[axis];
    points[2][right] = grid.min[right] + deltaR * s;
    points[2][up] = grid.min[up] + deltaU * (t + 1.0f/grid.size);

    points[3][axis] = grid.max[axis];
    points[3][right] = grid.min[right] + deltaR * (s + 1.0f/grid.size);
    points[3][up] = grid.min[up] + deltaU * (t);
    // Define points[4] to points[7]

    // points[4]
    points[4][axis] = grid.min[axis];
    points[4][right] = grid.min[right] + deltaR * (u + 1.0f/grid.size);
    points[4][up] = grid.min[up] + deltaU * v;

    // points[5]
    points[5][axis] = grid.min[axis];
    points[5][right] = grid.min[right] + deltaR * u;
    points[5][up] = grid.min[up] + deltaU * (v + 1.0f/grid.size);

    // points[6]
    points[6][axis] = grid.max[axis];
    points[6][right] = grid.min[right] + deltaR * (s + 1.0f/grid.size);
    points[6][up] = grid.min[up] + deltaU * (t + 1.0f/grid.size);

    // points[7]
    points[7][axis] = grid.max[axis];
    points[7][right] = grid.min[right] + deltaR * s;
    points[7][up] = grid.min[up] + deltaU * t;

        // Edges of the cuboid
    Vector3 edges[12] = {
        points[4] - points[0],
        points[5] - points[0],
        points[3] - points[4],
        points[1] - points[4],
        points[3] - points[7],
        points[0] - points[7],
        points[1] - points[5],
        points[6] - points[1],
        points[3] - points[6],
        points[2] - points[6],
        points[5] - points[2],
        points[2] - points[7],
    };



    // transform each triangle in local space and test it against a unit cube
    int startIdx = grid.indicies.size();

    if (!grid.hasTris) {
        testChannelAgainstAABB(grid, axis, up, right, points, edges, indicies);
    } else {
        testChannelAgainstTriangles(grid, axis, up, right, points, edges, indicies);
    }
    int lutIdx = getLUTIdx(u, v, s, t, idx, isObject);
    int endIdx = grid.indicies.size();
    grid.gridLutStart[lutIdx] = startIdx;
    grid.gridLutEnd[lutIdx] = endIdx;
}
