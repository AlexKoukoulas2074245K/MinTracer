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
	vec3& normalize()
	{
		T nor2 = length2();
		if (nor2 > 0) {
			T invNor = 1 / sqrt(nor2);
			x *= invNor, y *= invNor, z *= invNor;
		}
		return *this;
	}
	vec3<T> operator * (const T& f) const { return vec3<T>(x * f, y * f, z * f); }
	vec3<T> operator * (const vec3<T>& v) const { return vec3<T>(x * v.x, y * v.y, z * v.z); }
	T dot(const vec3<T>& v) const { return x * v.x + y * v.y + z * v.z; }
	vec3<T> cross(const vec3<T>& v) const { return vec3<T>(y * v.z - z * v.y, z * v.x - v * v.z, x * v.y - y * v.x); }
	vec3<T> operator - (const vec3<T>& v) const { return vec3<T>(x - v.x, y - v.y, z - v.z); }
	vec3<T> operator + (const vec3<T>& v) const { return vec3<T>(x + v.x, y + v.y, z + v.z); }
	vec3<T>& operator += (const vec3<T>& v) { x += v.x, y += v.y, z += v.z; return *this; }
	vec3<T>& operator *= (const vec3<T>& v) { x *= v.x, y *= v.y, z *= v.z; return *this; }
	vec3<T> operator - () const { return vec3<T>(-x, -y, -z); }
	T length2() const { return x * x + y * y + z * z; }
	T length() const { return sqrt(length2()); }
	friend std::ostream& operator << (std::ostream& os, const vec3<T>& v)
	{
		os << "[" << v.x << " " << v.y << " " << v.z << "]";
		return os;
	}
};

template<typename T>
inline uint32 vec3toARGB(const vec3<T>& vec)
{
	return 0xFF000000 |
		uint32(min(1.0f, vec.x) * 255) << 16 |
		uint32(min(1.0f, vec.y) * 255) << 8 |
		uint32(min(1.0f, vec.z) * 255);
}

