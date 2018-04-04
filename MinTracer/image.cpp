/**********************************************************************/
/** image.cpp by Alex Koukoulas (C) 2017 All Rights Reserved         **/
/** File Description: Implementation of Image class                  **/
/**********************************************************************/

#pragma once

// Local Headers
#include "image.h"

// Remote Headers
#include <fstream>

#pragma pack(push, 1)
struct BitmapHeader
{
	uint16 fileType;
	uint32 fileSize;
	uint16 reserved1;
	uint16 reserved2;
	uint32 bitmapOffset;
	uint32 size;
	sint32 width;
	sint32 height;
	uint16 planes;
	uint16 bitsPerPixel;
	uint32 compression;
	uint32 sizeOfBitmap;
	sint32 horResolution;
	sint32 verResolution;
	uint32 colorsUsed;
	uint32 colorsImportant;
};
#pragma pack(pop)


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

f32 Image::scale(const f32 scaleFactor)
{	
	const auto roundedScaleFactor = scaleFactor > 1.0f ? roundf(scaleFactor) : (scaleFactor > 0.4f ? 0.5f : 0.25f);
	
	// Determine the target dimensions of the scaled image, be it upscaled or downscaleed
	std::vector<std::vector<vec3<f32>>> resultData;
	const auto resultWidth = static_cast<sint32>(_width * roundedScaleFactor);
	const auto resultHeight = static_cast<sint32>(_height * roundedScaleFactor);
	resultData.resize(resultHeight, std::vector<vec3<f32>>(resultWidth, 0.0f));	
	
	const auto invScaleFactor = 1.0f / roundedScaleFactor;

	// Downscaling (2x2 averaging)
	if (invScaleFactor > 1.0f)
	{
		const auto step = lroundf(invScaleFactor);
		const auto weight = 1.0f / lroundf(static_cast<f32>(powl(step, 2)));

		for (auto y = step - 1; y < _height; y += step)
		{
			for (auto x = step - 1; x < _width; x += step)
			{
				for (auto j = -step + 1; j < 1; ++j)
				{
					for (auto i = -step + 1; i < 1; ++i)
					{
						resultData[y / step][x / step] += _data[y + j][x + i] * weight;
					}
				}
			}
		}		
	}
	// Upscaling (Nearest Neighbour)
	else
	{		
		
		for (auto y = 0; y < resultHeight; ++y)
		{
			for (auto x = 0; x < resultWidth; ++x)
			{								
				resultData[y][x] = _data[minu(_height - 1, y / static_cast<uint32>(roundedScaleFactor))]
					                    [minu(_width - 1, x / static_cast<uint32>(roundedScaleFactor))];
			}
		}
	}

	_data = resultData;
	_width = resultWidth;
	_height = resultHeight;

	return roundedScaleFactor;
}


void Image::writeToBMP(const std::string& fileName)
{	
	const auto outputPixelsSize = sizeof(uint32) * _width * _height;

	BitmapHeader bmh = {};
	bmh.fileType = 0x4D42;
	bmh.fileSize = sizeof(BitmapHeader) + static_cast<uint32>(outputPixelsSize);
	bmh.bitmapOffset = sizeof(BitmapHeader);
	bmh.size = sizeof(BitmapHeader) - 14;
	bmh.width = _width;
	bmh.height = -_height;
	bmh.planes = 1;
	bmh.bitsPerPixel = 32;
	bmh.sizeOfBitmap = static_cast<uint32>(outputPixelsSize);

	std::ofstream outputFile(fileName, std::ios::binary | std::ios::out);

	if (outputFile.good())
	{
		outputFile.write(reinterpret_cast<char*>(&bmh), sizeof(BitmapHeader));

		for (auto y = 0; y < _height; ++y)
		{
			for (auto x = 0; x < _width; ++x)
			{
				auto val = vec3toARGB(_data[y][x]);
				outputFile.write(reinterpret_cast<char*>(&val), sizeof(uint32));
			}
		}
	}

	outputFile.close();
}