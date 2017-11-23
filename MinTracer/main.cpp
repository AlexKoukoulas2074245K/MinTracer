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
#include <memory>
#include <vector>

#include "typedefs.h"
#include "math.h"

struct Material
{
	vec3<f32> diffuse;
	vec3<f32> specular;
};

struct Sphere
{
	f32 radius;
	vec3<f32> center;
	uint32 matIndex;
};

struct Ray
{
	vec3<f32> direction;
	vec3<f32> origin;
};

struct Light
{
	vec3<f32> position;
	vec3<f32> color;
};

struct Scene
{
	Sphere spheres[2];
	Light lights[1];
	Material materials[3];
};

struct HitInfo
{
	const bool hit;
	const vec3<f32> position;
	const vec3<f32> normal;
	const uint8 surfaceMatIndex;
	const f32 t;

	HitInfo(const bool hit, 
		    const vec3<f32>& position, 
		    const vec3<f32>& normal,
		    const uint8& surfaceMatIndex,
		    const f32 t)
		: hit(hit)
		, position(position)
		, normal(normal)
		, surfaceMatIndex(surfaceMatIndex)
		, t(t)
	{
	}
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

std::unique_ptr<HitInfo> raySphereIntersectionTest(const Ray& ray, const Sphere& sphere)
{
	const auto toRay = ray.origin - sphere.center;

	const auto a = ray.direction.dot(ray.direction);
	const auto b = 2.0f * ray.direction.dot(toRay);
	const auto c = toRay.dot(toRay) - sphere.radius * sphere.radius;
	const auto det = b * b - 4 * a * c;

	if (det > 0.0f)
	{
		f32 minT = (-b - sqrtf(det)) / 2 * a;
		f32 maxT = (-b + sqrtf(det)) / 2 * a;

		if (minT > 0.0f && minT < 10.0f)
		{
			vec3<f32> hitPos = ray.origin + ray.direction * minT;
			return std::make_unique<HitInfo>(true, hitPos, (hitPos - sphere.center).normalize(), sphere.matIndex, minT);
		}
		else if (maxT > 0.0f && maxT < 10.0f)
		{
			vec3<f32> hitPos = ray.origin + ray.direction * maxT;
			return std::make_unique<HitInfo>(true, hitPos, (hitPos - sphere.center).normalize(), sphere.matIndex, maxT);
		}		
	}
	
	return std::make_unique<HitInfo>(false, vec3<f32>(), vec3<f32>(), 0, 0.0f);
}

vec3<f32> trace(const Ray& ray, const Scene& scene)
{
	vec3<f32> fragment;

	for (auto sphere: scene.spheres)
	{
		const auto hitInfo = raySphereIntersectionTest(ray, sphere);

		if (hitInfo->hit)
		{
			vec3<f32> hitToLight = scene.lights[0].position - hitInfo->position;
			vec3<f32> lightDir = hitToLight.normalize();
			f32 diffuseTerm = max(0.0f, lightDir.dot(hitInfo->normal)); 
			return (scene.materials[hitInfo->surfaceMatIndex].diffuse * scene.lights[0].color) * diffuseTerm;
		}
	}

	return fragment;
}

void render(uint32* const pixels, const sint32 width, const sint32 height)
{
	const auto invWidth = 1.0f/width;
	const auto invHeight = 1.0f/height;
	const auto fov = PI/3.0f;
	const auto aspect = static_cast<f32>(width)/height;
	const auto angle = tan(fov * 0.5f);
	
	Scene scene = {};
	scene.lights[0] = Light{vec3<f32>(0.0f, 0.0f, -1.0f), vec3<f32>(0.7f, 0.7f, 0.7f)};

	scene.materials[0] = Material{vec3<f32>(0.0f, 0.0f, 0.0f), vec3<f32>(0.0f, 0.0f, 0.0f)};
	scene.materials[1] = Material{vec3<f32>(0.9f, 0.3f, 0.3f), vec3<f32>(0.9f, 0.3f, 0.3f)};
	scene.materials[2] = Material{vec3<f32>(0.3f, 0.5f, 0.9f), vec3<f32>(0.3f, 0.5f, 0.9f)};

	scene.spheres[0] = Sphere{2.0f, vec3<f32>(0.0f, 0.0f, -10.0f), 1};
	scene.spheres[1] = Sphere{3.7f, vec3<f32>(3.0f, 0.0f, -4.5f), 2};

	for (auto y = 0; y < height; ++y)
	{
		for (auto x = 0; x < width; ++x)
		{
			const auto xx = (2 * ((x + 0.5f) * invWidth) - 1) * angle * aspect;
			const auto yy = (1 - 2 * ((y + 0.5f) * invHeight)) * angle;
			
			vec3<f32> rayDirection(xx, yy, -1.0f);
			rayDirection.normalize();
			Ray ray = Ray{rayDirection, vec3<f32>()};

			const auto pixel = trace(ray, scene);
			pixels[y * width + x] = vec3toARGB(pixel);
		}

		const auto percComplete = uint32((y / static_cast<f32>(height)) * 100);

		if (percComplete % 10 == 0 && y % (height/10) == 0)
		{
			std::cout << "Tracing " << percComplete << "% complete" << std::endl;
		}		
	}

	std::cout << "Tracing 100% complete" << std::endl;
}

int main()
{
	// Output parameters
	const auto outputWidth = 640;
	const auto outputHeight = 480;
	const auto outputFileName = "output.bmp";

	auto* outputPixels = new uint32[outputHeight * outputWidth];
	
	render(outputPixels, outputWidth, outputHeight);
	
	writeBMP(outputFileName, outputPixels, outputWidth, outputHeight);
	
	std::cout << "Finished writing output to file.. " << std::endl;

	// Cleanup
	delete[] outputPixels;

	// Attempt to open result image 
	system(outputFileName);
	
}