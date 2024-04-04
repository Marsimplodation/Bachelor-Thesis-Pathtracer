#include "tracer.h"
#include "common.h"
#include "primitives/plane.h"
#include "primitives/triangle.h"
#include "scene/scene.h"
#include "shader/shader.h"
#include "types/camera.h"
#include "types/texture.h"
#include "types/vector.h"
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <thread>
#include "window/window.h"

namespace {
const int MAX_WIDTH = 4096;
const int MAX_HEIGHT = 2160;
int WIDTH = 800;
int HEIGHT = 600;
Texture image = createTexture(MAX_WIDTH, MAX_HEIGHT);
int samples[MAX_WIDTH][MAX_HEIGHT];
u32 randomStates[MAX_WIDTH][MAX_HEIGHT];
WaveFrontEntry wavefront[16];
int intersects[16];
std::thread threads[16];
bool running = true;
} // namespace

float getIntersectionCount() {
    float tests = 0.0f;
    for (int i = 0; i < 16; i++) {
        tests+=intersects[i];
    }
    return (float)tests / 1000000;
}

void traceWF(int i) {
    int x,y;
    Vector3 color{0,0,0};
    float xi1, xi2, xIn, yIn;
    while (running) {
        //if(onGoingReset) continue;
        y = wavefront[i].y;
        x = wavefront[i].x;
        auto &ray = wavefront[i].ray;
        if (ray.terminated) {
            ray.randomState = randomStates[x][y];
            xi1 = 2 * (fastRandom(ray.randomState)) - 1.0f;
            xi2 = 2 * (fastRandom(ray.randomState)) - 1.0f;
            xIn = 2.0f * ((float)(x + xi1) / (float)WIDTH) - 1.0f;
            yIn = 2.0f * ((float)(y + xi2) / (float)HEIGHT) - 1.0f;
            yIn = -yIn;
            if (WIDTH >= HEIGHT)
                yIn = (float)HEIGHT / (float)WIDTH * yIn;
            else
                xIn = (float)WIDTH / float(HEIGHT) * xIn;
            color = {0.0f, 0.0f, 0.0f};
            ray.throughPut = {1.0f, 1.0f, 1.0f};
            ray.depth = 0;
            createCameraRay(xIn, yIn, ray);
        }

        findIntersection(ray);
        // float t = ray.throughPut;
        color += shade(ray);
        ray.depth++;

        if (!ray.terminated)
            continue;
        bool xOverFlow = x + 4 >= WIDTH;
        bool yOverFlow = y + 4 >= HEIGHT;
        if (xOverFlow && yOverFlow) {
            intersects[i] = ray.interSectionTests;
            ray.interSectionTests = 0;
            wavefront[i].x = x + 4 - WIDTH;
            wavefront[i].y = y + 4 - HEIGHT;
        } else if (xOverFlow) {
            wavefront[i].x = x + 4 - WIDTH;
            wavefront[i].y = y + 4;
        } else {
            wavefront[i].x += 4;
        }
        float currentSample = (float)samples[x][y];
        color += tracerGetPixel(x, y) * (float)currentSample;
        color = color / (currentSample + 1.0f);
        
        setPixel(x, y, color);
        samples[x][y]++;
        randomStates[x][y] = ray.randomState;
    }
}

void trace() {
    for (int i = 0; i < 16; i++) {
        threads[i] = std::thread(std::bind(traceWF, i));
    }
    for (int i = 0; i < 16; i++) {
        threads[i].join();
    }
}

Vector3 tracerGetPixel(int x, int y) {
    return getTextureRGBAt(image, x, y);
}

void setPixel(int x, int y, Vector3 &c) {
    setTextureAt(image, x, y, c);
}

void reset() {
    Vector3 v{0, 0, 0};
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            setTextureAt(image, x, y, {});
            samples[x][y] = 0;
        }
    }
    for (int i = 0; i < 16; i++) {
        intersects[i] = 0;
        wavefront[i].x = i % 4;
        wavefront[i].y = fmax(0.0f, i - (i % 4)) / 4;
        wavefront[i].ray.terminated = true;
    }
}

void setWindowSize(int x, int y) {
    WIDTH = x;
    HEIGHT = y;
    reset();
}

Vector3 getWindowSize() { return {(float)WIDTH, (float)HEIGHT, 0.0f}; }

void initTracer() {
    for (int i = 0; i < 16; i++) {
        wavefront[i].x = i % 4;
        wavefront[i].y = fmax(0.0f, i - (i % 4)) / 4;
        wavefront[i].ray.terminated = true;
    }
    for (int x = 0; x < MAX_WIDTH; x++) {
        for (int y = 0; y < MAX_HEIGHT; y++) {
            setTextureAt(image, x, y, {});

            samples[x][y] = 0;
            randomStates[x][y] = hashCoords(x, y); 
        }
    }
    initScene();
}
void destroyTracer() {
    running = false;
    for (int i = 0; i < 16; i++) {
        wavefront[i].ray.terminated = true;
    }
    destroyScene();
}
