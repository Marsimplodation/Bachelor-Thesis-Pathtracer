#ifndef TRACER_H
#define TRACER_H
#include "types/ray.h"
//#define WIDTH 800
//#define HEIGHT 600
#define EPSILON 0.001
#include "types/vector.h"

struct WaveFrontEntry {
    Ray ray;
    int x;
    int y;
};


void trace(bool testing);
void initTracer();
void destroyTracer();
Vector3 tracerGetPixel(int x, int y);
void setPixel(int x, int y, Vector3 &c);
void reset();
void setWindowSize(int x, int y);
Vector3 getWindowSize();
unsigned long getIntersectionCount(); 
unsigned long getStructureIntersectionCount();

int& getMaxSampleCount();
bool& getfinishedRendering(); 
bool& getDebugView();
bool& getDebugShowTris();
int& getDebugScale();
#endif // !TRACER_H
