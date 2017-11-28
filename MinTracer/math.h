/********************************************************************/
/** math.h by Alex Koukoulas (C) 2017 All Rights Reserved          **/
/** File Description:                                              **/
/********************************************************************/

#pragma once
#include <cmath>
#include "typedefs.h"

const f32 PI = 3.1415927f;

template<typename T>
inline T min(const T& a, const T& b) { return a < b ? a : b; }

template<typename T>
inline T max(const T& a, const T& b) { return a > b ? a : b; }

template<typename T>
class vec3
{
public:
	T x, y, z;
	vec3() : x(T(0)), y(T(0)), z(T(0)) {}
	vec3(T xx) : x(xx), y(xx), z(xx) {}
	vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
	
	vec3<T>& operator -= (const vec3<T>& v) { x -= v.x, y -= v.y; z -= v.z; return *this; }
	vec3<T>& operator += (const vec3<T>& v) { x += v.x; y += v.y; z += v.z; return *this; }
	vec3<T>& operator *= (const vec3<T>& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
	vec3<T>& operator /= (const vec3<T>& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }

	vec3<T>& operator *= (const T& scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }
	vec3<T>& operator /= (const T& scalar) { x /= scalar; y /= scalar; z /= scalar; return *this; }
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
		uint32(min(1.0f, vec.x) * 255) << 16 |
		uint32(min(1.0f, vec.y) * 255) << 8 |
		uint32(min(1.0f, vec.z) * 255);
}

