#include "tracer.h"
#include "window/window.h"
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <thread>

void ttrace() {trace();}

int main(int argc, char **argv) {
    initTracer();
    auto draw = std::thread(createWindow);
    auto traceT = std::thread(ttrace);

    
    for (float t = 0.0f; ; t += 0.001f) { // Increment t to animate the colors
        //getCamera()->origin.z -= 0.1f*std::sin(t);
    }

    draw.join();
    traceT.join();
    destroyTracer();
    return 0;
}
