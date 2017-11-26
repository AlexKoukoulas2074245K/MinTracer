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
#include "image.h"

static const f32 T_MIN = 0.01f;
static const f32 T_MAX = 100.0f;

struct Material
{
	vec3<f32> ambient;
	vec3<f32> diffuse;
	vec3<f32> specular;
	f32 glossiness;

	Material(const vec3<f32>& ambient, const vec3<f32>& diffuse, const vec3<f32>& specular, f32 glossiness)
		: ambient(ambient)
		, diffuse(diffuse)
		, specular(specular)
		, glossiness(glossiness)
	{
	}
};

struct Sphere
{
	f32 radius;
	vec3<f32> center;
	uint32 matIndex;

	Sphere(const f32 radius, const vec3<f32>& center, const uint32 matIndex)
		: radius(radius)
		, center(center)
		, matIndex(matIndex)
	{
	}
};

struct Plane
{
	vec3<f32> normal;
	f32 d;
	uint32 matIndex;

	Plane(const vec3<f32>& normal, const f32 d, const uint32 matIndex)
		: normal(normal)
		, d(d)
		, matIndex(matIndex)
	{
	}
};

struct Ray
{
	vec3<f32> direction;
	vec3<f32> origin;

	Ray(const vec3<f32>& direction, const vec3<f32>& origin)
		: direction(direction)
		, origin(origin)
	{
	}
};

struct Light
{
	vec3<f32> position;
	vec3<f32> color;

	Light(const vec3<f32>& position, const vec3<f32>& color)
		: position(position)
		, color(color)
	{
	}
};

struct Scene
{
	std::vector<Sphere> spheres;
	std::vector<Light> lights;
	std::vector<Material> materials;
	std::vector<Plane> planes;
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

void scaleImage(const Image<vec3<f32>>& inputImage, Image<vec3<f32>>& result, const f32 scaleFactor)
{
	const auto width = inputImage.getWidth();
	const auto height = inputImage.getHeight();

	result.resize(static_cast<sint32>(width * scaleFactor), static_cast<sint32>(height * scaleFactor));

	const auto roundedScaleFactor = lroundf(scaleFactor);
	const auto invScaleFactor = 1.0f / scaleFactor;
	
	// Downscaling (2x2 averaging)
	if (invScaleFactor > 1.0f)
	{
		const auto step = lroundf(invScaleFactor);
		const auto weight = 1.0f / lroundf(static_cast<float>(powl(step, 2)));

		for (auto y = step - 1; y < height; y += step)
		{
			for (auto x = step - 1; x < width; x += step)
			{
				
				for (auto j = -step + 1; j < 1; ++j)
				{
					for (auto i = -step + 1; i < 1; ++i)
					{
						result[y/step][x/step] += inputImage[y + j][x + i] * weight;
					}
				}								
			}
		}
	}
	// Upscaling (Nearest Neighbour)
	else	
	{
		const auto width = result.getWidth();
		const auto height = result.getHeight();

		for (auto y = 0; y < height; ++y)
		{
			for (auto x = 0; x < width; ++x)
			{
				result[y][x] = inputImage[y / roundedScaleFactor][x / roundedScaleFactor];
			}
		}
	}
}

void writeBMP(const std::string& fileName, const Image<vec3<f32>>& inputImage)
{
	const auto width = inputImage.getWidth();
	const auto height = inputImage.getHeight();
	const auto outputPixelsSize = sizeof(uint32) * width * height;
	
	BitmapHeader bmh = {};
	bmh.fileType     = 0x4D42;
	bmh.fileSize     = sizeof(BitmapHeader) + static_cast<uint32>(outputPixelsSize);
	bmh.bitmapOffset = sizeof(BitmapHeader);
	bmh.size         = sizeof(BitmapHeader) - 14;
	bmh.width        = width;
	bmh.height       = -height;
	bmh.planes       = 1;
	bmh.bitsPerPixel = 32;
	bmh.sizeOfBitmap = static_cast<uint32>(outputPixelsSize);

	std::ofstream outputFile(fileName, std::ios::binary | std::ios::out);

	if (outputFile.good())
	{
		outputFile.write(reinterpret_cast<char*>(&bmh), sizeof(BitmapHeader));		

		for (auto y = 0; y < height; ++y)
		{
			for (auto x = 0; x < width; ++x)
			{
				auto val = vec3toARGB(inputImage[y][x]);
				outputFile.write(reinterpret_cast<char*>(&val), sizeof(uint32));
			}
		}		
	}

	outputFile.close();
}

std::unique_ptr<HitInfo> rayPlaneIntersectionTest(const Ray& ray, const Plane& plane)
{
	const auto denom = (-ray.direction).dot(plane.normal);

	if (denom > 0.0f)
	{
		const auto t = (plane.d - (plane.normal.dot(ray.origin)))/denom;
		const auto hitPos = ray.origin + ray.direction * t;
		
		return std::make_unique<HitInfo>(true, hitPos, plane.normal, plane.matIndex, t);
	}

	return std::make_unique<HitInfo>(false, vec3<f32>(), vec3<f32>(), 0, T_MAX);
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
		const auto minT = (-b - sqrtf(det)) / 2 * a;
		const auto maxT = (-b + sqrtf(det)) / 2 * a;

		if (minT > 0.0f)
		{
			const auto hitPos = ray.origin + ray.direction * (minT);
			auto normal = (hitPos - sphere.center).normalize();

			if (toRay.length() < sphere.radius + T_MIN)
			{
				normal = -normal;
			}

			return std::make_unique<HitInfo>(true, hitPos, (hitPos - sphere.center).normalize(), sphere.matIndex, minT);
		}
		
	}
	
	return std::make_unique<HitInfo>(false, vec3<f32>(), vec3<f32>(), 0, T_MAX);
}

std::unique_ptr<HitInfo> intersectScene(const Scene& scene, const Ray& ray)
{
	std::unique_ptr<HitInfo> closestHitInfo = std::make_unique<HitInfo>(false, vec3<f32>(), vec3<f32>(), 0, T_MAX);
	
	for (auto sphere : scene.spheres)
	{
		auto hitInfo = raySphereIntersectionTest(ray, sphere);

		if (hitInfo->hit && hitInfo->t < closestHitInfo->t)
		{			
			closestHitInfo = std::move(hitInfo);
		}
	}

	for (auto plane : scene.planes)
	{
		auto hitInfo = rayPlaneIntersectionTest(ray, plane);

		if (hitInfo->hit && hitInfo->t < closestHitInfo->t)
		{			
			closestHitInfo = std::move(hitInfo);
		}
	}

	return closestHitInfo;
}

vec3<f32> shade(const Scene& scene, const Ray& ray, const Light& light)
{
	vec3<f32> colorAccum;
	
	const auto hitInfo = intersectScene(scene, ray);

	if (hitInfo->hit)
	{		
		auto hitToLight = light.position - hitInfo->position;
		const auto lightDir = hitToLight.normalize();
		const auto viewDir = (hitInfo->position - ray.origin).normalize();
		const auto reflDir = (viewDir - hitInfo->normal * viewDir.dot(hitInfo->normal) * 2.0f).normalize();

		const auto diffuseTerm = max(0.0f, lightDir.dot(hitInfo->normal));
		const auto& material = scene.materials[hitInfo->surfaceMatIndex];
		const auto specularTerm = powf(max(0.0f, lightDir.dot(reflDir)), material.glossiness);
		colorAccum += (material.diffuse * light.color) * diffuseTerm;
		colorAccum += (material.specular * light.color) * specularTerm;	

		auto visibility = 1.0f;
		auto lightHitInfo = intersectScene(scene, Ray(hitToLight, hitInfo->position));

		if (lightHitInfo->hit)
		{			
			const auto originalHitToLightMag = (light.position - hitInfo->position).length();
			const auto reverseIntersectionHitToLightMag = (light.position - lightHitInfo->position).length();

			visibility = (originalHitToLightMag - reverseIntersectionHitToLightMag) > 0.001f ? 0.0f : 1.0f;		
			colorAccum *= visibility;
		}

		colorAccum += material.ambient;
	}
	return colorAccum;
}

vec3<f32> trace(const Ray& ray, const Scene& scene)
{
	vec3<f32> fragment;

	for (auto light : scene.lights)
	{
		fragment += shade(scene, ray, light);
	}	

	return fragment;
}

void render(Image<vec3<f32>>& result)
{
	// Initialize scene
	Scene scene = {};
	    
	scene.lights.emplace_back(vec3<f32>(-2.0f, 2.0f, -2.0f), vec3<f32>(0.7f, 0.7f, 0.7f));

	scene.materials.emplace_back(vec3<f32>(0.0f, 0.0f, 0.0f), vec3<f32>(0.0f, 0.0f, 0.0f), vec3<f32>(0.0f, 0.0f, 0.0f), 0.0f);
	scene.materials.emplace_back(vec3<f32>(0.3f, 0.1f, 0.1f), vec3<f32>(0.9f, 0.3f, 0.3f), vec3<f32>(0.9f, 0.3f, 0.3f), 32.0f);
	scene.materials.emplace_back(vec3<f32>(0.1f, 0.2f, 0.4f), vec3<f32>(0.3f, 0.5f, 0.9f), vec3<f32>(0.3f, 0.5f, 0.9f), 64.0f);
	scene.materials.emplace_back(vec3<f32>(0.2f, 0.2f, 0.2f), vec3<f32>(0.5f, 0.5f, 0.5f), vec3<f32>(0.5f, 0.5f, 0.5f), 1.0f);
	scene.materials.emplace_back(vec3<f32>(0.1f, 0.1f, 0.4f), vec3<f32>(0.3f, 0.3f, 0.9f), vec3<f32>(0.3f, 0.3f, 0.9f), 24.0f);

	scene.spheres.emplace_back(2.0f, vec3<f32>(-3.0f, 1.0f, -9.0f), 1);
	scene.spheres.emplace_back(1.7f, vec3<f32>(2.3f, 0.0f, -9.0f), 2);
	scene.spheres.emplace_back(0.5f, vec3<f32>(0.0f, 0.0f, -5.0f), 4);

	scene.planes.emplace_back(vec3<f32>(0.0f, 0.0f, 1.0f), 30.0f, 3);
    scene.planes.emplace_back(vec3<f32>(0.0f, 1.0f, 0.0f), 2.0f, 3);	
	//scene.planes.emplace_back(vec3<f32>(-1.0f, 0.0f, 0.0f), 4.0f, 3);

	// Calculate ray direction parameters
	const auto width = result.getWidth();
	const auto height = result.getHeight();
	const auto invWidth = 1.0f / width;
	const auto invHeight = 1.0f / height;
	const auto fov = PI / 3.0f;
	const auto aspect = static_cast<f32>(width) / height;
	const auto angle = tan(fov * 0.5f);

	for (auto y = 0; y < height; ++y)
	{
		for (auto x = 0; x < width; ++x)
		{
			const auto xx = (2 * ((x + 0.5f) * invWidth) - 1) * angle * aspect;
			const auto yy = (1 - 2 * ((y + 0.5f) * invHeight)) * angle;
			
			vec3<f32> rayDirection(xx, yy, -1.0f);
			rayDirection.normalize();
			Ray ray(rayDirection, vec3<f32>());
			
			result[y][x] = trace(ray, scene);
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
	const auto outputFileName = "output.bmp";	
	const auto width = 64;
	const auto height = 48;

	// Render at initial resolution
    Image<vec3<f32>> renderedResult(width, height);
	render(renderedResult);

	// Scale Result
	Image<vec3<f32>> scaledResult;
    scaleImage(renderedResult, scaledResult, 8.0f);

	// Write Decimated Result
	writeBMP(outputFileName, scaledResult);
	
	std::cout << "Finished writing output to file.. " << std::endl;

	// Attempt to open result image 
	system(outputFileName);
	
}