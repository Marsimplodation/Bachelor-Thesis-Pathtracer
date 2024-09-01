#include "lightFieldGridSubBeams.h"
#include "accelerationStructures/lightFieldGrid.h"
#include "common.h"
#include "scene/scene.h"
#include "types/sat.h"
#include "types/vector.h"
#include <cstdint>
#include <unordered_set>
#include <vector>

namespace {
    std::vector<u32> indicies;
    GridSubBeams grids[] = {{.splitingAxis = 0}, {.splitingAxis = 1}, {.splitingAxis = 2}};
    u32 gridSize = 10;
    u32 maxTrisCount = 40;
    std::vector<Beam> beams;

    //helper functions
    inline int getLUTIdx(float u, float v, float s, float t) {
        u = (int) (u*gridSize);
        v = (int) (v*gridSize);
        s = (int) (s*gridSize);
        t = (int) (t*gridSize);
        return (u * gridSize * gridSize * gridSize) + (v * gridSize * gridSize) + (s * gridSize) + t;
    }
    inline int getINTLUTIdx(int u, int v, int s, int t) {
        return (u * gridSize * gridSize * gridSize) + (v * gridSize * gridSize) + (s * gridSize) + t;
    }
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
}


void intersectSubBeamGrid(Ray &r) {
    auto &objectBuffer = getObjects();
    auto &indicieBuffer = getIndicies();
    auto &trisBuffer = getTris();
   
    int axis = 0, idx = 0;
    float maxDelta = 0.0f;
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
    idx = axis;
    auto & grid = grids[axis];
    auto axes = getGridAxes(axis);
    int right = axes[0];
    int up = axes[1];


    r.interSectionAS++;
    float d1 = (grid.min[axis] - r.origin[axis]) * r.inv_dir[axis];
    float d2 = (grid.max[axis] - r.origin[axis]) * r.inv_dir[axis];
    bool gridMiss = std::max(d1, d2) < 0;
    if (gridMiss) return; 

    r.interSectionAS++;
    if(!findIntersection(r, grid.aabb)) return;
   
    //get intersection point
    auto in = (r.origin + d1 * r.direction - grid.min) / (grid.max - grid.min);
    auto out = (r.origin + d2 * r.direction - grid.min) / (grid.max - grid.min);

    // ray is in channel uv,st
    // to do get all tris in the lut for uvst and loop over them
    float lutIdx = getLUTIdx(in[right], in[up], out[right], out[up]);


    std::vector<u32> toTraverse(0);
    toTraverse.clear();
    toTraverse.push_back(grid.beams[lutIdx]);
    Vector4 points{};
    u32 iter = 0;
    float u = in[right];
    float v = in[up];
    float s = out[right];
    float t = out[up];
    while(toTraverse.size() > 0) {
        if(iter++ >= 300) return;
        if(r.terminated) return;
        u32 beamIdx = toTraverse.back();
        Beam & beam = beams[beamIdx];
        toTraverse.pop_back();
        
        //perform grid interesection
        if(beam.startIdx >= beam.endIdx) continue;
        if(beam.hasTris) {
            for (unsigned int i = beam.startIdx; i < beam.endIdx; ++i) {
                u32 sIdx = indicies[i];
                r.interSectionTests++;
                Triangle &triangle = trisBuffer[sIdx];
                triangleIntersection(r, triangle);
            }
                return;
        } else { 
            //ensure the closest aabb is traversed first (this is a stack, so it has to be last in the loop)
            for (unsigned int i = beam.startIdx; i < beam.endIdx; ++i) {
                auto &child = beams[i];                
                auto min = child.minUVST/(float)gridSize;
                auto max = child.maxUVST/(float)gridSize;
                if(
                    u < min[0] || v < min[1] || s < min[2] || t < min[3] ||
                    u > max[0] || v > max[1] || s > max[2] || t > max[3]
                ) continue;
                else toTraverse.push_back(i);
            }
        }
    }
}

u64 getMemoryGridBeams() {
    auto beamsSize = beams.size() * sizeof(Beam);
    auto gridsSize = sizeof(GridSubBeams)*3 + (grids[0].beams.size()  + grids[1].beams.size() + grids[2].beams.size()) * sizeof(u32);
    auto indiciesSize = indicies.size() * sizeof(u32);
    return beamsSize + gridSize + indiciesSize;
}

void setGridBeamsSettings(u32 size, u32 count) {
    gridSize = size;
    maxTrisCount = count;
}

void adjustGridSize(int idx) {
    GridSubBeams &grid = grids[idx];
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


void constructBeam(u32 beamIdx, int gridIdx) {
    std::vector<u32> beamsToBuild;
    beamsToBuild.push_back(beamIdx);
    auto & grid = grids[gridIdx];
    const auto axis = grid.splitingAxis;
    const auto axes = getGridAxes(axis);
    const int right = axes[0];
    const int up = axes[1];
    float deltaR = grid.max[right] - grid.min[right];
    float deltaU = grid.max[up] - grid.min[up];
    while(beamsToBuild.size() != 0) {
        beamIdx = beamsToBuild.back();
        beamsToBuild.pop_back();

        auto & beam = beams[beamIdx];
        if(beam.indiciesForChilds.size() > 0) {
            beam.indiciesForChilds.clear();
            continue;
        }

        auto maxUVST = beam.maxUVST / (float) gridSize;
        auto minUVST = beam.minUVST / (float )gridSize;
        
        Vector3 points[8];
        Vector3 edges[12];
        // the alorithm needs 4 points
        // choosen points:
        // front left bottom u,v
        // from right up u+1,v+1
        // back left up s,t+1
        // back right bottom s+1,t
        // the rest of the 4 points are used when comparing with the object aabb
        
        points[0][axis] = grid.min[axis];
        points[0][right] = grid.min[right] + deltaR * minUVST.x;
        points[0][up] = grid.min[up] + deltaU * minUVST.y;

        points[1][axis] = grid.min[axis];
        points[1][right] = grid.min[right] + deltaR * (maxUVST.x);
        points[1][up] = grid.min[up] + deltaU * (maxUVST.y);

        points[2][axis] = grid.max[axis];
        points[2][right] = grid.min[right] + deltaR * minUVST.z;
        points[2][up] = grid.min[up] + deltaU * (maxUVST.w);

        points[3][axis] = grid.max[axis];
        points[3][right] = grid.min[right] + deltaR * (maxUVST.z);
        points[3][up] = grid.min[up] + deltaU * minUVST.w;
        // Define points[4] to points[7]

        // points[4]
        points[4][axis] = grid.min[axis];
        points[4][right] = grid.min[right] + deltaR * (maxUVST.x);
        points[4][up] = grid.min[up] + deltaU * minUVST.y;

        // points[5]
        points[5][axis] = grid.min[axis];
        points[5][right] = grid.min[right] + deltaR * minUVST.x;
        points[5][up] = grid.min[up] + deltaU * (maxUVST.y);

        // points[6]
        points[6][axis] = grid.max[axis];
        points[6][right] = grid.min[right] + deltaR * (maxUVST.z);
        points[6][up] = grid.min[up] + deltaU * (maxUVST.w);

        // points[7]
        points[7][axis] = grid.max[axis];
        points[7][right] = grid.min[right] + deltaR * minUVST.z;
        points[7][up] = grid.min[up] + deltaU * minUVST.w;
        
        
        edges[0] =  points[4] - points[0];
        edges[1] =  points[5] - points[0];
        edges[2] =  points[3] - points[4];
        edges[3] =  points[1] - points[4];
        edges[4] =  points[3] - points[7];
        edges[5] =  points[0] - points[7];
        edges[6] =  points[1] - points[5];
        edges[7] =  points[6] - points[1];
        edges[8] =  points[3] - points[6];
        edges[9] =  points[2] - points[6];
        edges[10] =  points[5] - points[2];
        edges[11] =  points[2] - points[7];
        for(int i=0; i < 12; ++i) normalize(edges[i]);

        std::vector<u32> inBeam;
        if(beam.parentIdx == UINT32_MAX) {
            auto & trisBuffer = getTris();
            for(int trisIdx = 0; trisIdx < trisBuffer.size(); ++trisIdx) {
                if(!triInChannel(trisBuffer[trisIdx].vertices, points, edges)) continue;
                inBeam.push_back(trisIdx);
            }
        } else {
            auto & trisBuffer = getTris();
            auto & parentNode = beams[beam.parentIdx];
            for(auto trisIdx : parentNode.indiciesForChilds) {
                if(!triInChannel(trisBuffer[trisIdx].vertices, points, edges)) continue;
                inBeam.push_back(trisIdx);
            }

        }

        if(inBeam.size() < maxTrisCount) {
            auto startIdx = indicies.size();
            for(auto idx : inBeam) indicies.push_back(idx);
            auto endIdx = indicies.size();
            beam.startIdx = startIdx;
            beam.endIdx = endIdx;
            beam.hasTris = true;
        } else {
            auto steps = gridSize;
            auto deltaUVST = (beam.maxUVST-beam.minUVST);
            auto stepSize = 1.0f/steps;

            beam.hasTris = false;
            beamsToBuild.push_back(beamIdx);
            for(auto idx : inBeam) beam.indiciesForChilds.push_back(idx);
            //create all 16 combinations of min and max indexable by 0 - 3
            auto startIdx = beams.size() - 1;

            std::vector<Vector4> combinations(steps*steps*steps*steps);
            for (int i = 0, iter=0; i < steps; ++i) {
                for (int j = 0; j < steps; ++j) {
                    for (int k = 0; k < steps; ++k) {
                        for (int h = 0; h < steps; ++h) {
                            combinations[iter++] = {
                                beam.minUVST[0] + i * stepSize * deltaUVST[0],
                                beam.minUVST[1] + j * stepSize * deltaUVST[1],
                                beam.minUVST[2] + k * stepSize * deltaUVST[2],
                                beam.minUVST[3] + h * stepSize * deltaUVST[3],
                            };
                        }
                    }
                }
            }
            for (int i = 0; i < combinations.size(); ++i) {
                Vector4 combination = combinations[i];;
                Vector4 max = combination + stepSize * deltaUVST;
                beams.push_back(Beam{.minUVST=combination, .maxUVST=max, .parentIdx = beamIdx, .indiciesForChilds = {}});
                u32 bIdx = beams.size() -1;
                beamsToBuild.push_back(bIdx);
            }
            auto & beam = beams[beamIdx];
            auto endIdx = beams.size();
            beam.startIdx = startIdx;
            beam.endIdx = endIdx;
        }
    }
}


void buildGridsWithSubBeams() {
    float count = gridSize * gridSize * gridSize * gridSize;
    beams.reserve(gridSize*gridSize*3);
    for(int axis = 0; axis < 3; ++axis) {
        grids[axis].beams.clear();
        grids[axis].beams.resize(count);
        grids[axis].min = getSceneMinBounds();
        grids[axis].max = getSceneMaxBounds();
        grids[axis].aabb = {.min = grids[axis].min, .max = grids[axis].max};

        // offset the min and max based on the camera
        adjustGridSize(axis);
        
        printf("SubBeams %f GB\n", getMemoryGridBeams() / 1000000000.0f);
        printf("building channel LUT for Grid %d\n", axis);
        int i = 1;
        for (int u = 0; u < gridSize; u++) {
            for (int v = 0; v < gridSize; v++) {
                for (int s = 0; s < gridSize; s++) {
                    for (int t = 0; t < gridSize; t++) {
                        beams.push_back(
                            Beam{
                                .minUVST = {u+0.0f,v+0.0f,s+0.0f,t+0.0f},
                                .maxUVST{u+1.0f,v+1.0f,s+1.0f,t+1.0f},
                                .startIdx = 0,
                                .endIdx = 0,
                                .parentIdx = UINT32_MAX,
                                .indiciesForChilds = {}
                            });
                        
                        u32 LUTidx = getLUTIdx(u / (float) gridSize, v / (float) gridSize, s / (float) gridSize, t / (float) gridSize);
                        u32 beamsIdx = beams.size() -1;
                        grids[axis].beams[LUTidx] = beamsIdx;
                        constructBeam(beamsIdx, axis);
                    }
                }
            }
        }
        printf("SubBeams %f GB\n", getMemoryGridBeams() / 1000000000.0f);
    }
}
