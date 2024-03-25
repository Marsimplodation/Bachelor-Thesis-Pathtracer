#ifndef CAMERA_H
#define CAMERA_H
#include "ray.h"
#include "vector.h"
struct Camera {
    Vector3 origin;
    Vector3 forward;
    Vector3 right;
    Vector3 up;
    float focus;
    float fov;
}; 
Camera *getCamera();
void cameraSetForward(Vector3 & v);
void cameraSetUp(Vector3 & v);
void cameraSetFov(float f);
void createCameraRay(float x, float y, Ray &ray); 


#endif // !CAMERA_H
