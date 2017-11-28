/**********************************************************************/
/** image.cpp by Alex Koukoulas (C) 2017 All Rights Reserved  **/
/** File Description:                                                **/
/**********************************************************************/

#pragma once

// Local Headers
#include "image.h"

// Remote Headers


Image::Image()
{
}

Image::Image(const sint32 width, const sint32 height)
	: _width(width)
	, _height(height)
{
	resize(_width, _height);
}

void Image::resize(const sint32 width, const sint32 height)
{
	_data.resize(height, std::vector<vec3<f32>>(width, 0.0f)); 
	_width = width; 
	_height = height;
}