/**********************************************************************/
/** main.cpp by Alex Koukoulas (C) 2017 All Rights Reserved          **/
/** File Description:                                                **/
/**********************************************************************/

#if defined(DEBUG) || defined(_DEBUG)
#include <vld.h>
#include <io.h>
#include <fcntl.h>
#endif

#include <Windows.h>
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>

#include "typedefs.h"
#include "math.h"
#include "image.h"

static const f32 T_MIN = 0.01f;
static const f32 T_MAX = 100.0f;

using namespace std;

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
	vector<Sphere> spheres;
	vector<Light> lights;
	vector<Material> materials;
	vector<Plane> planes;
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

unique_ptr<HitInfo> rayPlaneIntersectionTest(const Ray& ray, const Plane& plane)
{
	const auto denom = dot(plane.normal, ray.direction);

	if (denom > 1e-6f || denom <-1e-6f)
	{
		const auto t = -(plane.d + (dot(plane.normal, ray.origin)))/denom;
		if (t > 0.0f)
		{
			const auto hitPos = ray.origin + ray.direction * t;
			return make_unique<HitInfo>(true, hitPos, plane.normal, plane.matIndex, t);						
		}
		
	}

	return make_unique<HitInfo>(false, vec3<f32>(), vec3<f32>(), 0, T_MAX);
}

unique_ptr<HitInfo> raySphereIntersectionTest(const Ray& ray, const Sphere& sphere)
{
	const auto toRay = ray.origin - sphere.center;

	const auto a = dot(ray.direction, ray.direction);
	const auto b = 2.0f * dot(toRay, ray.direction);
	const auto c = dot(toRay, toRay) - sphere.radius * sphere.radius;
	const auto det = b * b - 4 * a * c;

	if (det > 0.0f)
	{
		const auto minT = (-b - sqrtf(det)) / 2 * a;
		const auto maxT = (-b + sqrtf(det)) / 2 * a;

		if (minT > 0.0f)
		{
			const auto hitPos = ray.origin + ray.direction * minT;
			auto normal = normalize(hitPos - sphere.center);

			if (length(toRay) < sphere.radius + T_MIN)
			{
				normal = -normal;
			}

			return make_unique<HitInfo>(true, hitPos, normal, sphere.matIndex, minT);
		}
		
	}
	
	return make_unique<HitInfo>(false, vec3<f32>(), vec3<f32>(), 0, T_MAX);
}

unique_ptr<HitInfo> intersectScene(const Scene& scene, const Ray& ray)
{
	unique_ptr<HitInfo> closestHitInfo = make_unique<HitInfo>(false, vec3<f32>(), vec3<f32>(), 0, T_MAX);
	
	for (auto sphere : scene.spheres)
	{
		auto hitInfo = raySphereIntersectionTest(ray, sphere);

		if (hitInfo->hit && hitInfo->t < closestHitInfo->t)
		{			
			closestHitInfo = move(hitInfo);
		}
	}

	for (auto plane : scene.planes)
	{
		auto hitInfo = rayPlaneIntersectionTest(ray, plane);

		if (hitInfo->hit && hitInfo->t < closestHitInfo->t)
		{			
			closestHitInfo = move(hitInfo);
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
		hitToLight = normalize(hitToLight);
		const auto viewDir = normalize(hitInfo->position - ray.origin);
		const auto reflDir = normalize(viewDir - hitInfo->normal * dot(viewDir, hitInfo->normal) * 2.0f);

		const auto diffuseTerm = max(0.0f, dot(hitInfo->normal, hitToLight));
		const auto& material = scene.materials[hitInfo->surfaceMatIndex];
		const auto specularTerm = powf(max(0.0f, dot(reflDir, hitToLight)), material.glossiness);
		colorAccum += (material.diffuse * light.color) * diffuseTerm;
		colorAccum += (material.specular * light.color) * specularTerm;	
		
		auto lightHitInfo = intersectScene(scene, Ray(hitToLight, hitInfo->position));		

		// Shadow test
		if (lightHitInfo->hit)
		{																
			auto visibility = 1.0f;

			const auto prevHitToLight = light.position - hitInfo->position;
			const auto revHitToLight = light.position - lightHitInfo->position;
			const auto originalHitToLightMag = length(prevHitToLight);
			const auto reverseIntersectionHitToLightMag = length(revHitToLight);
			
			// In order to cancel visibility, i.e. the object is in shadow, we need to make sure that there
			// exists an object inbetween the original hit object and the light's position
			const auto objectInBetweenHitInfoAndLight = (originalHitToLightMag - reverseIntersectionHitToLightMag) > 1e-6f;			
			const auto objectNotBehindLight = dot(normalize(prevHitToLight), normalize(revHitToLight)) >= 1.0f - 1e-6f;

			// Enshadow only if the above conditions are satisfied
			visibility = objectInBetweenHitInfoAndLight && objectNotBehindLight ? 0.0f : 1.0f;
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

void render(const sint32 renderWidth, const sint32 renderHeight, const sint32 targetWidth, const sint32 targetHeight, HWND windowHandle, const string& outputFilename)
{
	// Initilize ray tracing result
	Image resultImage(renderWidth, renderHeight);

	// Initialize scene
	Scene scene = {};
	    
	scene.lights.emplace_back(vec3<f32>(3.0f, 1.0f, 0.0f), vec3<f32>(0.7f, 0.7f, 0.7f));

	scene.materials.emplace_back(vec3<f32>(0.0f, 0.0f, 0.0f), vec3<f32>(0.0f, 0.0f, 0.0f), vec3<f32>(0.0f, 0.0f, 0.0f), 0.0f);
	scene.materials.emplace_back(vec3<f32>(0.3f, 0.1f, 0.1f), vec3<f32>(0.9f, 0.3f, 0.3f), vec3<f32>(0.9f, 0.3f, 0.3f), 32.0f);
	scene.materials.emplace_back(vec3<f32>(0.1f, 0.2f, 0.4f), vec3<f32>(0.3f, 0.5f, 0.9f), vec3<f32>(0.3f, 0.5f, 0.9f), 64.0f);
	scene.materials.emplace_back(vec3<f32>(0.2f, 0.2f, 0.2f), vec3<f32>(0.5f, 0.5f, 0.5f), vec3<f32>(0.5f, 0.5f, 0.5f), 1.0f);
	scene.materials.emplace_back(vec3<f32>(0.1f, 0.1f, 0.4f), vec3<f32>(0.3f, 0.3f, 0.9f), vec3<f32>(0.3f, 0.3f, 0.9f), 24.0f);	

	scene.spheres.emplace_back(2.0f, vec3<f32>(-3.0f, 1.0f, -9.0f), 1);
	scene.spheres.emplace_back(1.7f, vec3<f32>(2.3f, 0.0f, -9.0f), 2);
	scene.spheres.emplace_back(0.5f, vec3<f32>(0.0f, 0.0f, -5.0f), 4);

	scene.planes.emplace_back(vec3<f32>(0.0f, 0.0f, 1.0f), 10.0f, 3);
    scene.planes.emplace_back(vec3<f32>(0.0f, 1.0f, 0.0f), 2.0f, 3);		
	scene.planes.emplace_back(vec3<f32>(0.0f, -1.0f, 0.0f), 4.0f, 3);
	scene.planes.emplace_back(vec3<f32>(-1.0f, 0.0f, 0.0f), 4.0f, 3);
	scene.planes.emplace_back(vec3<f32>(1.0f, 0.0f, 0.0f), 4.0f, 3);

	// Calculate ray direction parameters	
	const auto invWidth = 1.0f / renderWidth;
	const auto invHeight = 1.0f / renderHeight;
	const auto fov = PI / 3.0f;
	const auto aspect = static_cast<f32>(renderWidth) / renderHeight;
	const auto angle = tan(fov * 0.5f);

	const auto threadCount = 4;	
	const auto renderStart = chrono::steady_clock::now();

	atomic_long rowsRendered = 0;
	thread workers[threadCount];
	thread announcer([&rowsRendered, renderHeight]()
	{
		auto currentPercent = 0;
		while (rowsRendered != renderHeight)
		{
			const auto completedPerc = static_cast<uint32>(100 * (static_cast<float>(rowsRendered) / renderHeight));
			if (currentPercent != completedPerc)
			{
				currentPercent = completedPerc;
				OutputDebugString(string("Ray Tracing " + to_string(currentPercent) + "% complete\n").c_str());
			}
		}
	});

	for (auto i = 0; i < threadCount; ++i)
	{
		const auto workPerThread = renderHeight / threadCount;
		auto currentThreadWork = workPerThread;

		if (i == threadCount - 1 && renderHeight % workPerThread != 0)
		{
			currentThreadWork += renderHeight % workPerThread;
		}

		workers[i] = thread([&scene, &resultImage, &rowsRendered, i, workPerThread, currentThreadWork, renderWidth, invWidth, invHeight, angle, aspect]()
		{
			for (auto y = i * workPerThread; y < i * workPerThread + currentThreadWork; ++y)
			{
				for (auto x = 0; x < renderWidth; ++x)
				{
					const auto xx = (2 * ((x + 0.5f) * invWidth) - 1) * angle * aspect;
					const auto yy = (1 - 2 * ((y + 0.5f) * invHeight)) * angle;
					
					vec3<f32> rayDirection(xx, yy, -1.0f);
					rayDirection = normalize(rayDirection);
					Ray ray(rayDirection, vec3<f32>());

					resultImage[y][x] = trace(ray, scene);					
				}
				rowsRendered++;
			}
		});
	}

	for (auto i = 0; i < threadCount; ++i)
	{
		workers[i].join();
	}	
	announcer.join();

	const auto diff = chrono::steady_clock::now() - renderStart;
	cout << "Ray Trace finished - " << chrono::duration<double, milli>(diff).count() << " ms elapsed | " << threadCount << " with worker(s)" << endl;	


	// Scale result
	resultImage.scale((targetWidth + targetHeight) / static_cast<f32>(renderWidth + renderHeight));

	// Create bitmap array
	COLORREF *arr = (COLORREF*)calloc(targetWidth * targetHeight, sizeof(COLORREF));

	for (auto y = 0; y < targetHeight; ++y)
	{
		for (auto x = 0; x < targetWidth; ++x)
		{
			const auto colorVec = resultImage.getPixel(x, y);
			arr[y * targetWidth + x] = RGB(minf(1.0f, colorVec.x) * 255, minf(1.0f, colorVec.y) * 255, minf(1.0f, colorVec.z) * 255);
		}
	}

	// Creating and display bitmap
	HBITMAP map = CreateBitmap(targetWidth, targetHeight, 1, 8 * 4, (void*)arr);
	HDC src = CreateCompatibleDC(GetDC(windowHandle));
	SelectObject(src, map); 
	BitBlt(GetDC(windowHandle), 0, 0, targetWidth, targetHeight, src, 0, 0, SRCCOPY);
	DeleteDC(src); // Deleting temp HDC
	free(arr);

	// Write result to file
	resultImage.writeToBMP(outputFilename);

	cout << "Finished writing output to file.. " << endl;


	// Attempt to open result image 
	//system(outputFilename.c_str());
}

LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		} break;	
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND createWindow(HINSTANCE instance, const sint32 windowWidth, const sint32 windowHeight, const string& title)
{
	// Window Registration
	WNDCLASS wc = {};
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hInstance = instance;
	wc.lpfnWndProc = windowProc;
	wc.lpszClassName = title.c_str();
	wc.lpszMenuName = 0;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClass(&wc))
	{
		MessageBox(0, "Window class registration failed", 0, MB_ICONERROR);
		PostQuitMessage(-1);
	}

	// Adjust window to fit client rect 
	RECT rect = { 0, 0, windowWidth, windowHeight };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

	const auto w = rect.right - rect.left;
	const auto h = rect.bottom - rect.top;

	const auto x = (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2;
	const auto y = (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2;

	auto handle = CreateWindow(title.c_str(), title.c_str(), WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME, x, y, w, h, 0, 0, instance, 0);

	if (!handle)
	{
		MessageBox(0, "Window creation failed", "Failure", 0);
		PostQuitMessage(-1);
	}

	// Show and Update window
	ShowWindow(handle, SW_SHOW);
	UpdateWindow(handle);

	return handle;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE __, LPSTR ___, int ____)
{	
	// Window, Render & Target Parameters
	const auto windowWidth = 640;
	const auto windowHeight = 480;
	const auto targetWidth = windowWidth * 4;
	const auto targetHeight = windowHeight * 4;
	auto renderWidth = 80;
	auto renderHeight = 60;

	auto windowHandle = createWindow(instance, windowWidth, windowHeight, "MinTracer");	
	
	MSG msg = {};
	
	auto rendering = false;

	while (msg.message != WM_QUIT)
	{
		if (msg.message == WM_PAINT)
		{		
			if (!rendering && renderWidth <= targetWidth)
			{
				rendering = true;
				thread masterRayTraceThread([&rendering, &renderWidth, &renderHeight, windowHeight, windowWidth, targetWidth, targetHeight, windowHandle]()
				{
					render(renderWidth, renderHeight, windowWidth, windowHeight, windowHandle, "output_images/last_rendering_" + to_string(renderWidth) + "x" + to_string(renderHeight) + ".bmp");
					SetWindowText(windowHandle, ("MinTracer -- Current resolution: " + to_string(renderWidth) + " x " + to_string(renderHeight)).c_str());
					if (renderWidth <= targetWidth)
					{
						renderWidth *= 2;
						renderHeight *= 2;						
					}
					rendering = false;

				});

				masterRayTraceThread.detach();
			}			
		}

		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (renderWidth <= targetWidth && !rendering)
		{
			InvalidateRect(windowHandle, NULL, TRUE);
		}
	}		

	return 0;
}