#pragma once
#ifndef VEC3_H
#define VEC3_H


template <typename T>
class tvec3
{
    public:
    T x, y, z;

    // Constructor
    tvec3() : x() , y() , z() {}
    tvec3(const tvec3<T> &vec) : x(vec.x) , y(vec.y) , z(vec.z) {}
    tvec3(T X, T Y, T Z) : x(X) , y(Y) , z(Z) {}

    tvec3<T>& operator= (const tvec3<T>&);

    // Operators (Some are overloaded)
    tvec3<T>& operator* (const tvec3<T>&);
    tvec3<T>& operator/ (const tvec3<T>&);
    tvec3<T>& operator+ (const tvec3<T>&);
    tvec3<T>& operator- (const tvec3<T>&);
    tvec3<T>& operator* (T);
    tvec3<T>& operator/ (T);
    tvec3<T>& operator+ (T);
    tvec3<T>& operator- (T);

    tvec3<T>& operator*= (const tvec3<T>&);
    tvec3<T>& operator/= (const tvec3<T>&);
    tvec3<T>& operator+= (const tvec3<T>&);
    tvec3<T>& operator-= (const tvec3<T>&);
    tvec3<T>& operator*= (T);
    tvec3<T>& operator/= (T);
    tvec3<T>& operator+= (T);
    tvec3<T>& operator-= (T);
};

namespace Math {
    template <typename T> tvec3<T> CrossProduct(const tvec3<T>& v1, const tvec3<T>& v2);
    template <typename T> T DotProduct(const tvec3<T>& v1, const tvec3<T>& v2);
    template <typename T> T Magnitude(const tvec3<T>& v);
    template <typename T> void  Normalize(tvec3<T>& v);
    //void        Rotate(const tvec3<T>&);
    //void        Multiply(const mat44<T>&);
}; // namespace Math


typedef tvec3<float> vec3;
typedef tvec3<int32_t> ivec3;

#endif //__VEC3_H__
