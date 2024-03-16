#include "tracer.h"
#include "primitives/plane.h"
#include "primitives/triangle.h"
#include "scene/scene.h"
#include "shader/shader.h"
#include "types/camera.h"
#include "types/vector.h"
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <thread>

namespace {
const int MAX_WIDTH = 4096;
const int MAX_HEIGHT = 2160;
int WIDTH = 800;
int HEIGHT = 600;
Vector3 *pixels;
int *samples;
WaveFrontEntry wavefront[16];
std::thread threads[16];
bool onGoingReset = false;
} // namespace

void traceWF(int i) {
    while (true) {
        if (onGoingReset)
            continue;
        int y = wavefront[i].y;
        int x = wavefront[i].x;
        auto &ray = wavefront[i].ray;
        if (ray.terminated) {
            float xi1 = 2 * ((float)rand() / (float)RAND_MAX) - 1.0f;
            float xi2 = 2 * ((float)rand() / (float)RAND_MAX) - 1.0f;
            float xIn = 2.0f * ((float)(x + xi1) / (float)WIDTH) - 1.0f;
            float yIn = 2.0f * ((float)(y + xi2) / (float)HEIGHT) - 1.0f;
            yIn = -yIn;
            if (WIDTH >= HEIGHT)
                yIn = (float)HEIGHT / (float)WIDTH * yIn;
            else
                xIn = (float)WIDTH / float(HEIGHT) * xIn;

            ray.color = {0.0f, 0.0f, 0.0f};
            ray.colorMask = {1.0f, 1.0f, 1.0f};
            ray.throughPut = 1.0f;
            ray.depth = 0;
            createCameraRay(xIn, yIn, ray);
        }

        findIntersection(ray);
        // float t = ray.throughPut;
        auto color = shade(ray);
        ray.color += (color);
        ray.depth++;

        if (!ray.terminated)
            continue;
        bool xOverFlow = x + 4 >= WIDTH;
        bool yOverFlow = y + 4 >= HEIGHT;
        if (xOverFlow && yOverFlow) {
            wavefront[i].x = x + 4 - WIDTH;
            wavefront[i].y = y + 4 - HEIGHT;
        } else if (xOverFlow) {
            wavefront[i].x = x + 4 - WIDTH;
            wavefront[i].y = y + 4;
        } else {
            wavefront[i].x += 4;
        }
        auto pixelValue = tracerGetPixel(x, y);
        float currentSample = (float)samples[y * WIDTH + x];
        pixelValue = pixelValue * (float)currentSample;
        pixelValue += ray.color;
        pixelValue = pixelValue / (currentSample + 1.0f);
        pixelValue = clampToOne(pixelValue);
        setPixel(x, y, pixelValue);
        samples[y * WIDTH + x]++;
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
    if (onGoingReset)
        return {0, 0, 0};
    if (x >= WIDTH || x < 0 || y >= HEIGHT || y < 0)
        return {};

    return pixels[y * WIDTH + x];
}

void setPixel(int x, int y, Vector3 &c) {
    if (onGoingReset)
        return;
    if (x >= WIDTH || x < 0 || y >= HEIGHT || y < 0)
        return;
    pixels[y * WIDTH + x].x = c.x;
    pixels[y * WIDTH + x].y = c.y;
    pixels[y * WIDTH + x].z = c.z;
}

void reset() {
    onGoingReset = true;
    Vector3 v{0, 0, 0};
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            pixels[y * WIDTH + x].x = v.x;
            pixels[y * WIDTH + x].y = v.y;
            pixels[y * WIDTH + x].z = v.z;

            samples[y * WIDTH + x] = 0;
        }
    }
    for (int i = 0; i < 16; i++) {
        wavefront[i].x = i % 4;
        wavefront[i].y = fmax(0.0f, i - (i % 4)) / 4;
        wavefront[i].ray.terminated = true;
    }
    onGoingReset = false;
}

void setWindowSize(int x, int y) {
    onGoingReset = true;
    WIDTH = x;
    HEIGHT = y;
    reset();
    onGoingReset = false;
}

Vector3 getWindowSize() { return {(float)WIDTH, (float)HEIGHT, 0.0f}; }

void initTracer() {
    pixels = (Vector3 *)malloc(sizeof(Vector3) * MAX_WIDTH * MAX_HEIGHT);
    samples = (int *)malloc(sizeof(int) * MAX_WIDTH * MAX_HEIGHT);
    for (int i = 0; i < 16; i++) {
        wavefront[i].x = i % 4;
        wavefront[i].y = fmax(0.0f, i - (i % 4)) / 4;
        wavefront[i].ray.terminated = true;
    }
    initScene();
}
void destroyTracer() {
    free(pixels);
    free(samples);
}
