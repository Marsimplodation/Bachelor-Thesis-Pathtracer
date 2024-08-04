#include "common.h"
#include "scene/SceneFile.h"
#include "scene/scene.h"
#include "tracer.h"
#include "window/window.h"
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <thread>

namespace {
    bool testing = false;
}
void ttrace() {trace(testing);}
void twindow() {createWindow(testing);}

int main(int argc, char **argv) {
    if(argc != 3) return -1;
    testing = argv[2] == std::string("test");
    
    setWindowSize(1280, 720);
    getMaxSampleCount() = 10;
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
            draw.join();
            
            auto tNow = std::chrono::high_resolution_clock::now();
            float elapsed_time_ms = std::chrono::duration<double, std::milli>(tNow - tBegin).count();
            int seconds = (elapsed_time_ms) / 1000;
            int minutes = seconds / 60;
            seconds %= 60;
            int hours = minutes / 60;
            minutes %= 60;

            printf("Rendering Time: %02d:%02d:%02d\n", hours, minutes, seconds);
            printf("P Intersections: %lu \n", getIntersectionCount()); 
            printf("AS Intersections: %lu \n", getStructureIntersectionCount()); 
            printf("Intersections: %lu \n", getStructureIntersectionCount() + getIntersectionCount()); 

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
