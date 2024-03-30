#ifndef SCENE_H
#define SCENE_H
#include "../types/ray.h"
#include "../primitives/triangle.h"
#include <cstdlib>
#define KILLCHANCE 0.1
void findIntersection(Ray & r);
void findOcclusion(Ray & r);
void initScene();
void destroyScene();


template<typename T>
struct PrimitivesContainer{
    T* data;
    u32 count;
    u32 capacity;
};

template<typename T>
void addToPrimitiveContainer(PrimitivesContainer<T> & container, T primitive) {
    if (container.capacity == 0) {
        container.data = (T*) malloc(sizeof(T)*10);
        container.capacity = 10;
    } else if (container.capacity <= container.count) {
        container.data = (T*) realloc(container.data, sizeof(T)*(container.capacity + 10));
        container.capacity += 10;
    }
    container.data[container.count++] = primitive;
}

template<typename T>
void destroyContainer(PrimitivesContainer<T> & container) {
    if(!container.data) return;
    free(container.data);
}

Triangle *getObjectBufferAtIdx(int idx);
PrimitivesContainer<Triangle> *getObjectBuffer();

int getNumPrimitives();
void *getPrimitive(int idx);

void resetScene();
#endif // !SCENE_H
