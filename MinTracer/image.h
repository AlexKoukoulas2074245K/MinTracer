/********************************************************************/
/** image.h by Alex Koukoulas (C) 2017 All Rights Reserved         **/
/** File Description: Interface to the Image class, mostly used to **/
/** store and save constructed scenes                              **/
/********************************************************************/

#pragma once

// Local Headers
#include "typedefs.h"
#include "math.h"

// Remote Headers
#include <vector>

class Image
{
public:	
	Image();
	Image(const sint32 width, const sint32 height);
	
	inline const std::vector<vec3<f32>>& operator[] (const size_t i) const { return _data[i]; }
	inline std::vector<vec3<f32>>& operator[] (const size_t i) { return _data[i]; }

	inline sint32 getWidth() const { return _width; }
	inline sint32 getHeight() const { return _height; }
	inline vec3<f32> getPixel(const uint32 x, const uint32 y) const { return _data[y][x]; }
	inline void setPixel(const uint32 x, const uint32 y, const f32& val) { _data[y][x] = val; }
	
	void resize(const sint32 width, const sint32 height);
	f32 scale(const f32 scaleFactor);
	void writeToBMP(const std::string& fileName);

private:
	std::vector<std::vector<vec3<f32>>> _data;
	sint32 _width, _height;
};
