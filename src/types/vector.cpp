#include "vector.h"
#include "ray.h"
#include <cmath>

//--- Vector 3 ---//
float max(const Vector3 & v) {
    return fmaxf(v.x, fmaxf(v.y, v.z));
}

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

void operator*=(Vector3 & v, float s) {
    v = v * s;
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
        default: return -INFINITY; //blow up
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

//-- Vector 2 --//
float max(const Vector2 & v) {
    return fmaxf(v.x, v.y);
}

float dotProduct(const Vector2 &v1, const Vector2 &v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

float length(const Vector2 &v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

void normalize(Vector2 &v) {
    float l = length(v);
    v.x /= l;
    v.y /= l;
}

Vector2 normalized(const Vector2 &v) {
    Vector2 ret = v;
    normalize(ret);
    return ret;
}

Vector2 clampToOne(const Vector2 &v) {
    float max = fmaxf(v.x, v.y);
    if (max < 1.0f) max = 1.0f;
    return {
        .x = fminf(v.x / max, 1.0f),
        .y = fminf(v.y / max, 1.0f),
    };
}

Vector2 operator*(const Vector2 &v, float s) {
    return {
        .x = v.x * s,
        .y = v.y * s,
    };
}

Vector2 operator/(const Vector2 &v, float s) {
    return v * (1.0f / s);
}

Vector2 operator*(float s, const Vector2 &v) {
    return v * s;
}

void operator*=(Vector2 &v, float s) {
    v = v * s;
}

Vector2 operator-(const Vector2 &v1, const Vector2 &v2) {
    return {
        .x = v1.x - v2.x,
        .y = v1.y - v2.y,
    };
}

Vector2 operator+(const Vector2 &v1, const Vector2 &v2) {
    return {
        .x = v1.x + v2.x,
        .y = v1.y + v2.y,
    };
}

void operator+=(Vector2 &v1, const Vector2 &v2) {
    v1.x += v2.x;
    v1.y += v2.y;
}

float getIndex(const Vector2 &vec, int i) {
    switch (i) {
        case 0:
            return vec.x;
        case 1:
            return vec.y;
        default:
            return -INFINITY; // blow up
    }
}

void setIndex(Vector2 &vec, int i, float val) {
    switch (i) {
        case 0:
            vec.x = val;
            break;
        case 1:
            vec.y = val;
            break;
        default:
            break;
    }
}

//-- Vector 4 --//

float max(const Vector4 & v) {
    return fmaxf(v.x, fmaxf(v.y, fmaxf(v.z, v.w)));
}

float dotProduct(const Vector4 &v1, const Vector4 &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

float length(const Vector4 &v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

void normalize(Vector4 &v) {
    float l = length(v);
    v.x /= l;
    v.y /= l;
    v.z /= l;
    v.w /= l;
}

Vector4 normalized(const Vector4 &v) {
    Vector4 ret = v;
    normalize(ret);
    return ret;
}

Vector4 clampToOne(const Vector4 &v) {
    float max = fmaxf(v.x, fmaxf(v.y, fmaxf(v.z, v.w)));
    if (max < 1.0f) max = 1.0f;
    return {
        .x = fminf(v.x / max, 1.0f),
        .y = fminf(v.y / max, 1.0f),
        .z = fminf(v.z / max, 1.0f),
        .w = fminf(v.w / max, 1.0f),
    };
}

Vector4 operator*(const Vector4 &v, float s) {
    return {
        .x = v.x * s,
        .y = v.y * s,
        .z = v.z * s,
        .w = v.w * s,
    };
}

Vector4 operator/(const Vector4 &v, float s) {
    return v * (1.0f / s);
}

Vector4 operator*(float s, const Vector4 &v) {
    return v * s;
}

void operator*=(Vector4 &v, float s) {
    v = v * s;
}

Vector4 operator-(const Vector4 &v1, const Vector4 &v2) {
    return {
        .x = v1.x - v2.x,
        .y = v1.y - v2.y,
        .z = v1.z - v2.z,
        .w = v1.w - v2.w,
    };
}

Vector4 operator+(const Vector4 &v1, const Vector4 &v2) {
    return {
        .x = v1.x + v2.x,
        .y = v1.y + v2.y,
        .z = v1.z + v2.z,
        .w = v1.w + v2.w,
    };
}

void operator+=(Vector4 &v1, const Vector4 &v2) {
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
    v1.w += v2.w;
}

float getIndex(const Vector4 &vec, int i) {
    switch (i) {
        case 0:
            return vec.x;
        case 1:
            return vec.y;
        case 2:
            return vec.z;
        case 3:
            return vec.w;
        default:
            return -INFINITY; // blow up
    }
}

void setIndex(Vector4 &vec, int i, float val) {
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
        case 3:
            vec.w = val;
            break;
        default:
            break;
    }
}

