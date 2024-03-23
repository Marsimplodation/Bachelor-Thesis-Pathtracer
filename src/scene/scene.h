#ifndef SCENE_H
#define SCENE_H
#include "../types/ray.h"
#include "../primitives/triangle.h"
#include <cstdlib>
#define KILLCHANCE 0.50
void findIntersection(Ray & r);
void findOcclusion(Ray & r);
void initScene(float & scale);


template<typename T>
struct PrimitivesContainer{
    T* data;
    int count;
    int capacity;
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

Triangle *getObjectBufferAtIdx(int idx);
PrimitivesContainer<Triangle> *getObjectBuffer();

#endif // !SCENE_H
