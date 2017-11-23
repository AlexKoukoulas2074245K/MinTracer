/**********************************************************************/
/** main.cpp by Alex Koukoulas (C) 2017 All Rights Reserved          **/
/** File Description:                                                **/
/**********************************************************************/

#if defined(DEBUG) || defined(_DEBUG)
#include <vld.h>	
#endif

#include <fstream>
#include <iostream>
#include <string>

#include "typedefs.h"
#include "math.h"

struct Material
{
	vec3<f32> color;
};

struct Sphere
{
	f32 radius;
	vec3<f32> center;
};

struct Ray
{
	vec3<f32> direction;
	vec3<f32> origin;
	f32 t;
};


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

void writeBMP(const std::string& fileName, uint32* const outputPixels, const sint32 imageWidth, const sint32 imageHeight)
{
	const auto outputPixelsSize = sizeof(uint32) * imageWidth * imageHeight;
	
	BitmapHeader bmh = {};
	bmh.fileType     = 0x4D42;
	bmh.fileSize     = sizeof(BitmapHeader) + outputPixelsSize;
	bmh.bitmapOffset = sizeof(BitmapHeader);
	bmh.size         = sizeof(BitmapHeader) - 14;
	bmh.width        = imageWidth;
	bmh.height       = -imageHeight;
	bmh.planes       = 1;
	bmh.bitsPerPixel = 32;
	bmh.sizeOfBitmap = outputPixelsSize;

	std::ofstream outputFile(fileName, std::ios::binary | std::ios::out);

	if (outputFile.good())
	{
		outputFile.write(reinterpret_cast<char*>(&bmh), sizeof(BitmapHeader));
		outputFile.write(reinterpret_cast<char*>(outputPixels), outputPixelsSize);
	}

	outputFile.close();
}

vec3<f32> trace(const Ray& ray)
{
	Sphere s;
	s.radius = 2.0f;
	s.center = vec3<f32>(0.0f, 0.0f, -5.0f);

	f32 a = ray.direction.dot(ray.direction);
	f32 b = (ray.direction * 2.0f).dot(ray.origin - s.center);
	f32 c = (ray.origin - s.center).length2() - s.radius * s.radius;
	f32 det = b * b - 4 * a * c;

	f32 minT = (-b - sqrtf(det))/2 * a;
	f32 maxT = (-b + sqrtf(det))/2 * a;

	vec3<f32> hitPos = ray.origin + ray.direction * minT;

	if (minT > 0.0f || maxT > 0.0f)
	{
		return vec3<f32>(0.0f, 0.0f, 0.0f);
	}
	
	return vec3<f32>(1.0f, 1.0f, 1.0f);
}

void render(uint32* const pixels, const sint32 width, const sint32 height)
{
	const auto invWidth = 1.0f/width;
	const auto invHeight = 1.0f/height;
	const auto fov = PI/4.0f;
	const auto aspect = static_cast<f32>(width)/height;
	const auto angle = tan(PI * 0.5f * fov);

	for (auto y = 0; y < height; ++y)
	{
		for (auto x = 0; x < width; ++x)
		{
			const auto xx = (2 * ((x + 0.5f) * invWidth) - 1) * angle * aspect;
			const auto yy = (1 - 2 * ((y + 0.5f) * invHeight)) * angle;
			
			vec3<f32> rayDirection(xx, yy, -1.0f);
			rayDirection.normalize();
			Ray ray;
			ray.direction = rayDirection;			

			const auto pixel = trace(ray);
			pixels[y * width + x] = 0xFF000000 | 
				uint32(min(1.0f, pixel.x) * 255) << 16 |
			    uint32(min(1.0f, pixel.y) * 255) << 8 |
				uint32(min(1.0f, pixel.z) * 255);
		}
	}
}

int main()
{
	// Output parameters
	const auto outputWidth = 1280;
	const auto outputHeight = 720;
	const auto outputFileName = "output.bmp";

	auto* outputPixels = new uint32[outputHeight * outputWidth];
	
	render(outputPixels, outputWidth, outputHeight);
	
	writeBMP(outputFileName, outputPixels, outputWidth, outputHeight);

	// Cleanup
	delete[] outputPixels;

	// Attempt to open result image 
	system(outputFileName);

	std::cout << "Finished writing output to file.. " << std::endl;	
}