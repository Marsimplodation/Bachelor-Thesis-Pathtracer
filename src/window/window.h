#ifndef WINDOW_H
#define WINDOW_H
#include "../types/vector.h"

struct visualInformation {
    void * data;
    const char * name;
    char flag;
};

void createWindow(bool testing);
void registerInfo(float & f, const char * name);
void registerInfo(Vector3 & v, const char * name);
bool &getToneMapping();

#endif // !WINDOW_H
