/********************************************************************/
/** image.h by Alex Koukoulas (C) 2017 All Rights Reserved         **/
/** File Description:                                              **/
/********************************************************************/

#pragma once

// Local Headers
#include "typedefs.h"

// Remote Headers
#include <vector>

template <typename T>
class Image
{
public:
	Image(const sint32 width, const sint32 height)
		: _width(width)
		, _height(height)
	{
		_data.resize(height, std::vector<T>(width, T()));
	}
	
	inline const std::vector<T>& operator[] (const size_t i) const { return _data[i]; }
	inline std::vector<T>& operator[] (const size_t i) { return _data[i]; }

	inline sint32 getWidth() const { return _width; }
	inline sint32 getHeight() const { return _height; }
	inline T getPixel(const uint32 x, const uint32 y) const { return _data[y][x]; }
	inline void setPixel(const uint32 x, const uint32 y, const T& val) { _data[y][x] = val; }

private:
	std::vector<std::vector<T>> _data;
	sint32 _width, _height;
};
