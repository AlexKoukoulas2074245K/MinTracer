/********************************************************************/
/** math.h by Alex Koukoulas (C) 2017 All Rights Reserved          **/
/** File Description: Some helper math functions                   **/
/********************************************************************/

#pragma once
#include <cmath>
#include <sstream>

#include "typedefs.h"
#include "strutils.h"

const f32 PI = 3.1415927f;

inline f32 minf(const f32 a, const f32 b) { return a < b ? a : b; }
inline f32 maxf(const f32 a, const f32 b) { return a > b ? a : b; }

inline uint32 minu(const uint32 a, const uint32 b) { return a < b ? a : b; }
inline uint32 maxu(const uint32 a, const uint32 b) { return a > b ? a : b; }

template<typename T>
class vec3
{
public:
    T x, y, z;
    vec3() : x(T(0)), y(T(0)), z(T(0)) {}
    vec3(T xx) : x(xx), y(xx), z(xx) {}
    vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
    vec3(const std::string& desc) 
    {
        const auto vecDesc = strutils::split(desc, ','); 
        x = std::stof(vecDesc[0]);
        y = std::stof(vecDesc[1]);
        z = std::stof(vecDesc[2]); 
    }
    
    vec3<T>& operator -= (const vec3<T>& v) { x -= v.x, y -= v.y; z -= v.z; return *this; }
    vec3<T>& operator += (const vec3<T>& v) { x += v.x; y += v.y; z += v.z; return *this; }
    vec3<T>& operator *= (const vec3<T>& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
    vec3<T>& operator /= (const vec3<T>& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
    bool operator == (const vec3<T>& v) { return fabsf(x - v.x) < 1e-6 && fabsf(y - v.y) < 1e-6 && fabsf(z - v.z) < 1e-6; }
    bool operator != (const vec3<T>& v) { return !(*this == v); }
    vec3<T>& operator *= (const T& scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }
    vec3<T>& operator /= (const T& scalar) { x /= scalar; y /= scalar; z /= scalar; return *this; }

    std::string toString() const { std::stringstream result; result << x << "," << y << "," << z; return result.str(); }
};

template<typename T>
inline vec3<T> operator - (const vec3<T>& a) { return vec3<T>(-a.x, -a.y, -a.z); }

template<typename T>
inline vec3<T> operator + (const vec3<T>& a, const vec3<T>& b) { return vec3<T>(a.x + b.x, a.y + b.y, a.z + b.z); }

template<typename T>
inline vec3<T> operator - (const vec3<T>& a, const vec3<T>& b) { return vec3<T>(a.x - b.x, a.y - b.y, a.z - b.z); }

template<typename T>
inline vec3<T> operator * (const vec3<T>& a, const vec3<T>& b) { return vec3<T>(a.x * b.x, a.y * b.y, a.z * b.z); }

template<typename T>
inline vec3<T> operator / (const vec3<T>& a, const vec3<T>& b) { return vec3<T>(a.x / b.x, a.y / b.y, a.z / b.z); }

template<typename T>
inline vec3<T> operator * (const vec3<T>& a, const T& scalar) { return vec3<T>(a.x * scalar, a.y * scalar, a.z * scalar); }

template<typename T>
inline vec3<T> operator * (const T& scalar, const vec3<T>& a) { return a * scalar; }

template<typename T>
inline vec3<T> operator / (const vec3<T>& a, const T& scalar) { return vec3<T>(a.x / scalar, a.y / scalar, a.z / scalar); }

template<typename T>
inline vec3<T> operator / (const T& scalar, const vec3<T>& b) { return a / scalar; }

template<typename T>
inline T length(const vec3<T>& vec) { return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z); }

template<typename T>
inline vec3<T> normalize(const vec3<T>& vec) { return vec3<T>(vec / length(vec));  }

template<typename T>
inline T dot(const vec3<T>& a, const vec3<T>& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

template<typename T>
inline vec3<T> cross(const vec3<T>& a, const vec3<T>& b) { return vec3<T>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }

template<typename T>
inline uint32 vec3toARGB(const vec3<T>& vec)
{
    return 0xFF000000 |
        uint32(minf(1.0f, vec.x) * 255) << 16 |
        uint32(minf(1.0f, vec.y) * 255) << 8 |
        uint32(minf(1.0f, vec.z) * 255);
}

