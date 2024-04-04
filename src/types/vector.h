#ifndef VECTOR_H
#define VECTOR_H

struct Vector2 {
    float x;
    float y;
};

struct Vector3 {
    float x;
    float y;
    float z;
};

struct Vector4 {
    float x;
    float y;
    float z;
    float w;
};

//-- Vector 3 --//
float dotProduct(const Vector3 &v1, const Vector3 &v2);
float length(const Vector3 &v);
void normalize(Vector3 &v);
Vector3 normalized(const Vector3 &v);
Vector3 crossProduct(const Vector3 &v1, const Vector3 &v2);
void orthoNormalized(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3,
                     Vector3 *buff);
Vector3 clampToOne(const Vector3 & v);
Vector3 linearRGBToNonLinear(const Vector3 & v, float gamma);

Vector3 operator*(const Vector3 & v, float s);
Vector3 operator*(const Vector3 & v1, const Vector3 & v2);
Vector3 operator/(const Vector3 & v, float s);
Vector3 operator/(const Vector3 & v1,const Vector3 &v2);
Vector3 operator*(float s, const Vector3 & v);
void operator*=(Vector3 & v, float s);
Vector3 operator-(const Vector3 & v1, const Vector3 & v2);
Vector3 operator+(const Vector3 & v1, const Vector3 & v2);
void operator+=(Vector3 & v1, const Vector3 & v2);
float getIndex(const Vector3 & vec, int i);
void setIndex(Vector3 & vec, int i, float val);

//-- Vector 2 --//
float dotProduct(const Vector2 &v1, const Vector2 &v2);
float length(const Vector2 &v);
void normalize(Vector2 &v);
Vector2 normalized(const Vector2 &v);
Vector2 clampToOne(const Vector2 & v);

Vector2 operator*(const Vector2 &v, float s);
Vector2 operator/(const Vector2 &v, float s);
Vector2 operator*(float s, const Vector2 &v);
void operator*=(Vector2 &v, float s);
Vector2 operator-(const Vector2 &v1, const Vector2 &v2);
Vector2 operator+(const Vector2 &v1, const Vector2 &v2);
void operator+=(Vector2 &v1, const Vector2 &v2);
float getIndex(const Vector2 &vec, int i);
void setIndex(Vector2 &vec, int i, float val);

//-- Vector 4 --//
float dotProduct(const Vector4 &v1, const Vector4 &v2);
float length(const Vector4 &v);
void normalize(Vector4 &v);
Vector4 normalized(const Vector4 &v);
Vector4 clampToOne(const Vector4 & v);

Vector4 operator*(const Vector4 &v, float s);
Vector4 operator/(const Vector4 &v, float s);
Vector4 operator*(float s, const Vector4 &v);
void operator*=(Vector4 &v, float s);
Vector4 operator-(const Vector4 &v1, const Vector4 &v2);
Vector4 operator+(const Vector4 &v1, const Vector4 &v2);
void operator+=(Vector4 &v1, const Vector4 &v2);
float getIndex(const Vector4 &vec, int i);
void setIndex(Vector4 &vec, int i, float val);


#endif // !VECTOR_H
