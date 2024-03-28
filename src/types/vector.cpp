#include "vector.h"
#include "ray.h"
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


Vector3 clampToOne(const Vector3 & v){
    float max = fmaxf(v.x, fmaxf(v.y, v.z));
    if(max < 1.0f) max = 1.0f;
    return {
        .x = fminf(v.x/max, 1.0f),
        .y = fminf(v.y/max, 1.0f),
        .z = fminf(v.z/max, 1.0f),
    };
}

Vector3 linearRGBToNonLinear(const Vector3 & v, float gamma) {
    return {
        powf(v.x, gamma),
        powf(v.y, gamma),
        powf(v.z, gamma),
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

Vector3 operator/(const Vector3 & v1,const Vector3 &v2) {
    return {
        .x = v1.x / v2.x,
        .y = v1.y / v2.y,
        .z = v1.z / v2.z,
    };
}

float getIndex(const Vector3 & vec, int i) {
    switch (i) {
        case 0: return vec.x;
        case 1: return vec.y;
        case 2: return vec.z;
        default: return 0;
    }
}

void setIndex(Vector3 & vec, int i, float val) {
    switch (i) {
        case 0:
            vec.x = val;
            break;
        case 1:
            vec.y = val;
            break;
        case 2:
            vec.z = val;
            break;
        default: break;
    }

}
