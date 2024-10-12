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
    float fovScale;
    float dof;
    float pitch;
    float yaw;
    float speed;
    float sensitivity;
}; 
Camera *getCamera();
void cameraSetForward(Vector3 & v);
void cameraSetUp(Vector3 & v);
void cameraSetFov(float f);
void createCameraRay(float x, float y, Ray &ray); 
void rotate_camera(Vector2 mouse, float delta);
void move_camera(Vector2 keyboard,float delta);
#endif // !CAMERA_H
