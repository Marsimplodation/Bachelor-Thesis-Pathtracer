#include "tracer.h"
#include "common.h"
#include "scene/scene.h"
#include "shader/shader.h"
#include "types/camera.h"
#include "types/texture.h"
#include "types/vector.h"
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <mutex>
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
unsigned long intersectsP[16];
unsigned long intersectsA[16];
std::thread threads[16];
bool running = true;

int MAX_SAMPLE_COUNT = INT_MAX;
bool finishedRendering = false;
bool debugView = false;
bool debugViewTris = false;
int debugScale = 10;
bool testing = false;
} // namespace

bool& getfinishedRendering() {
    return finishedRendering;
}

bool& getDebugView() {
    return debugView;
}

bool& getDebugShowTris() {
    return debugViewTris;
}

int& getDebugScale() {
    return debugScale;
}

int& getMaxSampleCount() {
    return MAX_SAMPLE_COUNT;
}
unsigned long getIntersectionCount() {
unsigned long pTests = 0;
    for(auto num : intersectsP) pTests += num;
    return pTests;
}

unsigned long getStructureIntersectionCount() {
    unsigned long aTests = 0;
    for(auto num : intersectsA) aTests += num;
    return aTests;
}

void setupRay(Ray & ray, int x, int y) {
    
    float xi1, xi2, xIn, yIn;
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
    ray.throughPut = {1.0f, 1.0f, 1.0f};
    ray.depth = 0;
    createCameraRay(xIn, yIn, ray);
}

bool progressFront(Ray & ray,int i, int x, int y) {
    for(int step = 0; step < (WIDTH /4)*(HEIGHT/4); ++step) {
        bool xOverFlow = x + 4 >= WIDTH;
        bool yOverFlow = y + 4 >= HEIGHT;

        if (xOverFlow && yOverFlow) {
            ray.interSectionTests = 0;
            wavefront[i].x = x + 4 - WIDTH;
            wavefront[i].y = y + 4 - HEIGHT;
        } else if (xOverFlow) {
            wavefront[i].x = x + 4 - WIDTH;
            wavefront[i].y = y + 4;
        } else {
            wavefront[i].x += 4;
        }
        y = wavefront[i].y;
        x = wavefront[i].x;
        if(samples[x][y] < MAX_SAMPLE_COUNT) return true;
    }
    return false;
}

std::mutex testMtx;
void traceWF(int i) {
    int x,y;
    Vector3 color{0,0,0};
    while (running) {
        //if(onGoingReset) continue;
        y = wavefront[i].y;
        x = wavefront[i].x;
        auto &ray = wavefront[i].ray;
        if (ray.terminated) {
            setupRay(ray, x, y);
            color = {0.0f, 0.0f, 0.0f};
        }

        ray.interSectionAS = 0;
        ray.interSectionTests = 0;
        findIntersection(ray);
        // float t = ray.throughPut;
        intersectsP[i] += ray.interSectionTests;
        intersectsA[i] += ray.interSectionAS;
        color += shade(ray);
        ray.depth++;
            
        if(debugView) {
            auto it =(float) (debugViewTris ? ray.interSectionTests : ray.interSectionAS);
            if(it > debugScale) color = {1,0,0}; 
            else color = Vector3{it, it, it} / (float)debugScale;
            setPixel(x, y, color);
            ray.terminated = true;
        }
        if (!ray.terminated)
            continue;

        
        samples[x][y]++;
        randomStates[x][y] = ray.randomState;

        if(!debugView){
            float currentSample = (float)samples[x][y];
            color += tracerGetPixel(x, y) * (float)currentSample;
            color = color / (currentSample + 1.0f);
            setPixel(x, y, color);
        }
        
        
        if(!progressFront(ray, i, x, y)) break;
    }
}

std::mutex mtx;
bool reinitTracer = true;
void trace(bool testing) {
    bool run = true;
    ::testing = testing;
    while (run) {
        if(testing) run = false;
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (!reinitTracer) continue;
            reinitTracer = false;
        }

        for (int i = 0; i < 16; i++) {
            threads[i] = std::thread(std::bind(traceWF, i));
        }
        finishedRendering = false;
        for (int i = 0; i < 16; i++) {
            threads[i].join();
        }
        finishedRendering = true;
        if(!running) break;
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
    const int tmpS = MAX_SAMPLE_COUNT;
    MAX_SAMPLE_COUNT = 0;

    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            setTextureAt(image, x, y, {});
            samples[x][y] = 0;
        }
    }
    resetScene();
    for (int i = 0; i < 16; i++) {
        intersectsA[i] = 0;
        intersectsP[i] = 0;
        wavefront[i].x = i % 4;
        wavefront[i].y = fmax(0.0f, i - (i % 4)) / 4;
        wavefront[i].ray.terminated = true;
    }

    MAX_SAMPLE_COUNT = tmpS;
    reinitTracer = true;
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
        intersectsA[i] = 0;
        intersectsP[i] = 0;

    }
    for (int x = 0; x < MAX_WIDTH; x++) {
        for (int y = 0; y < MAX_HEIGHT; y++) {
            setTextureAt(image, x, y, {});
            samples[x][y] = 0;
            randomStates[x][y] = hashCoords(x, y); 
        }
    }
    reinitTracer = true;
}
void destroyTracer() {
    running = false;
    for (int i = 0; i < 16; i++) {
        wavefront[i].ray.terminated = true;
    }
    destroyScene();
}
