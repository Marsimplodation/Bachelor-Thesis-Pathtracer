#include "tracer.h"
#include "window/window.h"
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <thread>

void ttrace() {trace();}

int main(int argc, char **argv) {
    initTracer();
    auto draw = std::thread(createWindow);
    auto traceT = std::thread(ttrace);

    traceT.join();
    draw.join();
    return 0;
}
