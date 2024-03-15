#include "vector.h"
#include <cmath>

Vector3 crossProduct(const Vector3 &v1, const Vector3 &v2) {
    return {
        .x = v1.y * v2.z - v1.z * v2.y,
        .y = v1.z * v2.x - v1.x * v2.z,
        .z = v1.x * v2.y - v1.y * v2.x};
}

Vector3 normalized(const Vector3 &v) {
    Vector3 ret = v;
    normalize(ret);
    return ret;
}

float length(const Vector3 &v) {
    return std::sqrt(std::pow(v.x, 2) + std::pow(v.y, 2) + std::pow(v.z, 2));
}

void normalize(Vector3 &v) {
    float l = length(v);
    v.z /= l;
    v.x /= l;
    v.y /= l;
}

float dotProduct(const Vector3 &v1, const Vector3 &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

void orthoNormalized(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3,
                     Vector3 *buff) {
    buff[0] = normalized(v1);
    buff[1] = normalized(v2 - (dotProduct(buff[0], v2) * buff[0]));
    buff[2] = normalized(v3 - dotProduct(buff[0], v3) * buff[0] - dotProduct(buff[1], v3) * buff[1]);
}

Vector3 randomV3UnitHemisphere(Vector3 n) {
    Vector3 direction;
    float theta = (float)rand()/RAND_MAX; //=sinf(acosf(cos));
    float sinTheta = sqrtf(1-theta*theta);
    float phi = 2 * 3.14f * ((float)rand() / RAND_MAX);
    direction.x = sinTheta * cosf(phi);
    direction.y = sinTheta * sinf(phi);
    direction.z = theta;
    //adding the normal to attain the final random direction in the hemisphere
    if(dotProduct(n, direction) < 0.0f) direction = -1*direction;
    direction = normalized(direction + n);
  return direction;
}

Vector3 clampToOne(const Vector3 & v){
    return {
        fminf(v.x, 1.0f),
        fminf(v.y, 1.0f),
        fminf(v.z, 1.0f),
    };
}

//operators
Vector3 operator*(const Vector3 & v, float s) {
    return {
        .x = v.x * s,
        .y = v.y * s,
        .z = v.z * s,
    };
}

Vector3 operator*(float s, const Vector3 & v) {return v * s;}

Vector3 operator-(const Vector3 & v1, const Vector3 & v2) {
    return {
        .x = v1.x - v2.x,
        .y = v1.y - v2.y,
        .z = v1.z - v2.z,
    };
}

Vector3 operator+(const Vector3 & v1, const Vector3 & v2) {
    return {
        .x = v1.x + v2.x,
        .y = v1.y + v2.y,
        .z = v1.z + v2.z,
    };
}

Vector3 operator/(const Vector3 & v, float s) {return v * (1.0f/s);}
void operator+=(Vector3 & v1, const Vector3 & v2) {
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
}

Vector3 operator*(const Vector3 & v1, const Vector3 & v2) {
return {
        .x = v1.x * v2.x,
        .y = v1.y * v2.y,
        .z = v1.z * v2.z,
    };

}
