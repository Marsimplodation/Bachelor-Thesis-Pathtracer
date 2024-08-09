#include "common.h"
#include "scene/SceneFile.h"
#include "scene/scene.h"
#include "tracer.h"
#include "types/bvh.h"
#include "types/lightFieldGrid.h"
#include "window/window.h"
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <argparse.hpp>

namespace {
    bool testing = false;
}
void ttrace() {trace(testing);}
void twindow() {createWindow(testing);}

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("usage: pathtracer sceneFile {Samples} {gridSize} {objectGridSize} {maxTris}");
        return -1;
    }
    testing = argc != 2;
    
    u32 maxSamples = 100;
    u32 GridSize = 10;
    u32 GridObjectSize = 5;
    u32 GridMaxTris = 40;

    if(testing) {
        maxSamples = atoi(argv[2]);
        GridSize =  atoi(argv[3]);
        GridObjectSize = atoi(argv[4]);
        GridMaxTris = atoi(argv[5]);
    }
    setGridSettings(GridSize, GridObjectSize, GridMaxTris);

    setWindowSize(1280, 720);
    getMaxSampleCount() = maxSamples;
    loadScene(argv[1]);
    initScene();

    if(testing) {
        for (int i = 1; i < 3; i++) {
            auto tBegin = std::chrono::high_resolution_clock::now();
            setIntersectMode(i);
            initTracer();
            auto traceT = std::thread(ttrace);
            auto draw = std::thread(twindow);

            traceT.join();
            
            auto tNow = std::chrono::high_resolution_clock::now();
            float elapsed_time_ms = std::chrono::duration<double, std::milli>(tNow - tBegin).count();
            int seconds = (elapsed_time_ms) / 1000;
            int minutes = seconds / 60;
            seconds %= 60;
            int hours = minutes / 60;
            minutes %= 60;

            printf("Rendering Time: %f\n", elapsed_time_ms);
            printf("Triangle Intersections: %lu \n", getIntersectionCount()); 
            printf("AS Intersections: %lu \n", getStructureIntersectionCount()); 
            printf("Total Intersections: %lu \n", getStructureIntersectionCount() + getIntersectionCount()); 
            printf("Memory Consumption: %lu \n", (i == 1) ? getMemoryBVH() : getMemory2Plane()); 
            draw.join();

        }
    } else {
        initTracer();
        auto traceT = std::thread(ttrace);
        auto draw = std::thread(twindow);

        traceT.join();
        draw.join();
    }
    return 0;
}
