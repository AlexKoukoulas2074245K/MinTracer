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
	Image()
		: _width(0)
		, _height(0)
	{
		
	}

	Image(const sint32 width, const sint32 height)
		: _width(width)
		, _height(height)
	{
		resize(_width, _height);
	}
	
	inline const std::vector<T>& operator[] (const size_t i) const { return _data[i]; }
	inline std::vector<T>& operator[] (const size_t i) { return _data[i]; }

	inline sint32 getWidth() const { return _width; }
	inline sint32 getHeight() const { return _height; }
	inline T getPixel(const uint32 x, const uint32 y) const { return _data[y][x]; }
	inline void setPixel(const uint32 x, const uint32 y, const T& val) { _data[y][x] = val; }
	inline void resize(const sint32 width, const sint32 height) { _data.resize(height, std::vector<T>(width, T())); _width = width; _height = height; }

private:
	std::vector<std::vector<T>> _data;
	sint32 _width, _height;
};
