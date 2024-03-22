#ifndef VECTOR_H
#define VECTOR_H
struct Vector3 {
    float x;
    float y;
    float z;
};
float dotProduct(const Vector3 &v1, const Vector3 &v2);
float length(const Vector3 &v);
void normalize(Vector3 &v);
Vector3 normalized(const Vector3 &v);
Vector3 crossProduct(const Vector3 &v1, const Vector3 &v2);
void orthoNormalized(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3,
                     Vector3 *buff);
Vector3 randomV3UnitHemisphere(Vector3 n);
Vector3 clampToOne(const Vector3 & v);
Vector3 linearRGBToNonLinear(const Vector3 & v, float gamma);

Vector3 operator*(const Vector3 & v, float s);
Vector3 operator*(const Vector3 & v1, const Vector3 & v2);
Vector3 operator/(const Vector3 & v, float s);
Vector3 operator/(const Vector3 & v1,const Vector3 &v2);
Vector3 operator*(float s, const Vector3 & v);
Vector3 operator-(const Vector3 & v1, const Vector3 & v2);
Vector3 operator+(const Vector3 & v1, const Vector3 & v2);
void operator+=(Vector3 & v1, const Vector3 & v2);
#endif // !VECTOR_H
