#include "accelerationStructures/lightFieldGridSubBeams.h"
#include "common.h"
#include "scene/SceneFile.h"
#include "scene/scene.h"
#include "shader/shader.h"
#include "tracer.h"
#include "accelerationStructures/bvh.h"
#include "accelerationStructures/lightFieldGrid.h"
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

void printHelp() {
    printf("usage: pathtracer sceneFile");
    printf("Arguments: \n"
            "-S Samples default = 100 \n"
            "-gs gridSize defaul = 10 \n" 
            "-os objectGridSize default = 5 \n"
            "-m maxTris default = 40 \n" 
            "--testing  \n" 
            "--2plane_only \n"
            "--help\n");
}

int main(int argc, char **argv) {
    if(argc < 2) {
        printHelp();
        return -1;
    }
    
    u32 maxSamples = 100;
    u32 GridSize = 10;
    u32 GridObjectSize = 5;
    u32 GridMaxTris = 40;
    bool only2Plane = false;
    testing = false;
    auto checkNextArg =[argc](int i){return (i + 1 < argc);};
    for(int i = 0; i < argc; ++i){
        std::string arg(argv[i]);
        if(arg == "-S") {
            if(!checkNextArg(i)) {
                printHelp();
                return -1;
            }
            maxSamples = atoi(argv[++i]);
        } else if(arg == "-gs") {
            if(!checkNextArg(i)) {
                printHelp();
                return -1;
            }
            GridSize = atoi(argv[++i]);
        } else if(arg == "-ds") {
            if(!checkNextArg(i)) {
                printHelp();
                return -1;
            }
            getDebugScale() = atoi(argv[++i]);
        } else if(arg == "-os") {
            if(!checkNextArg(i)) {
                printHelp();
                return -1;
            }
            GridObjectSize = atoi(argv[++i]);
        } else if(arg == "-m") {
            if(!checkNextArg(i)) {
                printHelp();
                return -1;
            }
            GridMaxTris = atoi(argv[++i]);
        } else if(arg == "--testing") {
            testing = true;
        } else if(arg == "--2plane_only") {
            only2Plane = true;
        } else if(arg == "--help") {
            printHelp();
            return -1;
        }

    }

    setGridSettings(GridSize, GridObjectSize, GridMaxTris);
    setGridBeamsSettings(GridSize, GridMaxTris);

    setWindowSize(1280, 720);
    getMaxSampleCount() = maxSamples;
    loadScene(argv[1]);
    initScene();

    if(testing) {
        printf("starting test\n");
        getNEE() = false;
        for (int i = (only2Plane ? 2 : 1); i < 4; i++) {
            for (int j = 0; j < 3; ++j) {
                if(j==0) {
                    getMaxSampleCount() = maxSamples;
                    getDebugView() = false;
                    getToneMapping() = true;
                }
                else {
                    getToneMapping() = false;
                    getMaxSampleCount() = 1;
                    getDebugView() = true;
                    if(j==1) getDebugShowTris() = false;
                    if(j==2) getDebugShowTris() = true;
                }
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
                if(j==0) {
                printf("Rendering Time: %f\n", elapsed_time_ms);
                printf("Triangle Intersections: %lu \n", getIntersectionCount()); 
                printf("AS Intersections: %lu \n", getStructureIntersectionCount()); 
                printf("Total Intersections: %lu \n", getStructureIntersectionCount() + getIntersectionCount()); 
                printf("Memory Consumption: %lu \n", (i == 1) ? getMemoryBVH() : getMemory2Plane()); 
                }
                draw.join();
            }

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
