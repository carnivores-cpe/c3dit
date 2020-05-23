
#include "stdafx.h"
#include "vec3.h"
#include <cmath>


template <typename T>
tvec3<T>& tvec3<T>::operator= (const tvec3<T>& rhs)
{
    this->x = rhs.x;
    this->y = rhs.y;
    this->z = rhs.z;
    return *this;
}

template <typename T>
tvec3<T>& tvec3<T>::operator* (const tvec3<T> &vec)
{
    this->x *= vec.x;
    this->y *= vec.y;
    this->z *= vec.z;
    return *this;
}

template <typename T>
tvec3<T>& tvec3<T>::operator/ (const tvec3<T> &vec)
{
    this->x /= vec.x;
    this->y /= vec.y;
    this->z /= vec.z;
    return *this;
}

template <typename T>
tvec3<T>& tvec3<T>::operator+ (const tvec3<T> &vec)
{
    this->x += vec.x;
    this->y += vec.y;
    this->z += vec.z;
    return *this;
}

template <typename T>
tvec3<T>& tvec3<T>::operator- (const tvec3<T> &vec)
{
    this->x -= vec.x;
    this->y -= vec.y;
    this->z -= vec.z;
    return *this;
}

template <typename T>
tvec3<T>& tvec3<T>::operator* (T scal)
{
    this->x *= scal;
    this->y *= scal;
    this->z *= scal;
    return *this;
}

template <typename T>
tvec3<T>& tvec3<T>::operator/ (T scal)
{
    this->x /= scal;
    this->y /= scal;
    this->z /= scal;
    return *this;
}

template <typename T>
tvec3<T>& tvec3<T>::operator+ (T scal)
{
    this->x += scal;
    this->y += scal;
    this->z += scal;
    return *this;
}

template <typename T>
tvec3<T>& tvec3<T>::operator- (T scal)
{
    this->x -= scal;
    this->y -= scal;
    this->z -= scal;
    return *this;
}

template <typename T>
tvec3<T>& tvec3<T>::operator*= (const tvec3<T> &vec)
{
    this->x *= vec.x;
    this->y *= vec.y;
    this->z *= vec.z;
    return *this;
}

template <typename T>
tvec3<T>& tvec3<T>::operator/= (const tvec3<T> &vec)
{
    this->x /= vec.x;
    this->y /= vec.y;
    this->z /= vec.z;
    return *this;
}

template <typename T>
tvec3<T>& tvec3<T>::operator+= (const tvec3<T> &vec)
{
    this->x += vec.x;
    this->y += vec.y;
    this->z += vec.z;
    return *this;
}

template <typename T>
tvec3<T>& tvec3<T>::operator-= (const tvec3<T> &vec)
{
    this->x -= vec.x;
    this->y -= vec.y;
    this->z -= vec.z;
    return *this;
}

template <typename T>
tvec3<T>& tvec3<T>::operator*= (T scal)
{
    this->x *= scal;
    this->y *= scal;
    this->z *= scal;
    return *this;
}

template <typename T>
tvec3<T>& tvec3<T>::operator/= (T scal)
{
    this->x /= scal;
    this->y /= scal;
    this->z /= scal;
    return *this;
}

template <typename T>
tvec3<T>& tvec3<T>::operator+= (T scal)
{
    this->x += scal;
    this->y += scal;
    this->z += scal;
    return *this;
}

template <typename T>
tvec3<T>& tvec3<T>::operator-= (T scal)
{
    this->x -= scal;
    this->y -= scal;
    this->z -= scal;
    return *this;
}

template <typename T>
tvec3<T> Math::CrossProduct(const tvec3<T> &v1, const tvec3<T>& v2)
{
    // CrossProduct ( v1 x v2 ) Returns a Perpendicular Vector
    return tvec3<T>(
        v1.y * v2.z - v2.y * v1.z,
        v1.z * v2.x - v2.z * v1.x,
        v1.x * v2.y - v2.x * v1.y);
}

template <typename T>
T Math::DotProduct(const tvec3<T> &v1, const tvec3<T>& v2)
{
    // DotProduct ( v1 . v2 ) Returns the inner product
    return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}

template <typename T>
T Math::Magnitude(const tvec3<T>& v)
{
    // Length ( |v| ) Returns the length of this vector
    return std::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

template <typename T>
void Math::Normalize(tvec3<T>& v)
{
    // Convert to Unit Vector: [ v = v / |v| ]
    T m = Math::Magnitude( v );
    if (m != T()) {
        v.x = v.x / m;
        v.y = v.y / m;
        v.z = v.z / m;
    }
    else {
        v.x = T();
        v.y = T();
        v.z = T();
    }
}

/*template <typename T>
void tvec3<T>::Multiply(const mat44<T>& mat)
{
    this->x = (mat[0 ]*this->x) + (mat[1 ]*this->y) + (mat[2 ]*this->z);
    this->y = (mat[4 ]*this->x) + (mat[5 ]*this->y) + (mat[6 ]*this->z);
    this->z = (mat[8 ]*this->x) + (mat[9 ]*this->y) + (mat[10]*this->z);
}*/