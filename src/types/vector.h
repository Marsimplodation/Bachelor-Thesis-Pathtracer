#ifndef VECTOR_H
#define VECTOR_H

#include <cstddef>
#include <Eigen>

struct Vector2 {
    float x;
    float y;

      // Overload the [] operator for non-const objects
    float& operator[](size_t index) {
        switch (index) {
            case 0:
                return x;
            case 1:
                return y;
            default:
                return x;
        }
    }

     // Overload the [] operator for const objects
    const float& operator[](size_t index) const {
        switch (index) {
            case 0:
                return x;
            case 1:
                return y;
            default:
                return x;
        }
    }

};

struct Vector3 {
    Eigen::Vector3f vec;

    Vector3(float x, float y, float z) : vec(x, y, z) {}
    Vector3(const Vector3& v) : vec(v.vec) {}
    Vector3() : vec(0,0,0) {}
    Vector3(const Eigen::Vector3f& eigenVec) : vec(eigenVec) {}

    float& x = vec[0];
    float& y = vec[1];
    float& z = vec[2];

    float& operator[](size_t index) { return vec[index]; }
    const float& operator[](size_t index) const { return vec[index];}

    // Custom assignment operator to support initializer list
    Vector3& operator=(std::initializer_list<float> list) {
        if (list.size() == 3) {
            auto it = list.begin();
            vec[0] = *it++;
            vec[1] = *it++;
            vec[2] = *it;
        }
        return *this;
    }
    Vector3& operator=(const Vector3 & v){
        vec[0] = v.vec[0];
        vec[1] = v.vec[1];
        vec[2] = v.vec[2];
        return *this;
    }
// Arithmetic operators
    friend Vector3 operator*(const Vector3& v, float s) {
        return Vector3(v.vec * s);
    }

    void operator*=(float s) {
        vec *= s;
    }

    friend Vector3 operator*(float s, const Vector3& v) {
        return v * s;
    }

    friend Vector3 operator-(const Vector3& v1, const Vector3& v2) {
        return Vector3(v1.vec - v2.vec);
    }

    friend Vector3 operator+(const Vector3& v1, const Vector3& v2) {
        return Vector3(v1.vec + v2.vec);
    }

    friend Vector3 operator/(const Vector3& v, float s) {
        return Vector3(v.vec / s);
    }

    void operator+=(const Vector3& v) {
        vec += v.vec;
    }

    friend Vector3 operator*(const Vector3& v1, const Vector3& v2) {
        return Vector3(v1.vec.cwiseProduct(v2.vec));
    }

    friend Vector3 operator/(const Vector3& v1, const Vector3& v2) {
        return Vector3(v1.vec.cwiseQuotient(v2.vec));
    }

    // Other operations as needed
    
    // Example of dot product using Eigen's dot() function
    friend float dot(const Vector3& v1, const Vector3& v2) {
        return v1.vec.dot(v2.vec);
    }

    // Example of normalization using Eigen's normalized() function
    Vector3 normalized() const {
        return Vector3(vec.normalized());
    }

    // Example of length (magnitude) using Eigen's norm() function
    float norm() const {
        return vec.norm();
    }
};

struct Vector4 {
    float x;
    float y;
    float z;
    float w;
};

//-- Vector 3 --//
float max(const Vector3 & v, bool abs = false);
float dotProduct(const Vector3 &v1, const Vector3 &v2);
float length(const Vector3 &v);
void normalize(Vector3 &v);
Vector3 normalized(const Vector3 &v);
Vector3 crossProduct(const Vector3 &v1, const Vector3 &v2);
void orthoNormalized(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3,
                     Vector3 *buff);
Vector3 clampToOne(const Vector3 & v);

//-- Vector 2 --//
float max(const Vector2 & v);
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
float max(const Vector4 & v);
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
