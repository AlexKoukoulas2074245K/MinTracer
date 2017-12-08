/**********************************************************************/
/** main.cpp by Alex Koukoulas (C) 2017 All Rights Reserved          **/
/** File Description:                                                **/
/**********************************************************************/

#if defined(DEBUG) || defined(_DEBUG)
//#include <vld.h>
#endif

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>

#include "win32gui.h"
#include "scene.h"
#include "typedefs.h"
#include "math.h"
#include "image.h"

static const f32 T_MIN = 0.01f;
static const f32 T_MAX = 100.0f;

using namespace std;

struct HitInfo
{
	bool hit;
	vec3<f32> position;
	vec3<f32> normal;
	uint8 surfaceMatIndex;
	f32 t;

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

HitInfo rayPlaneIntersectionTest(const Ray& ray, const Plane& plane)
{
	const auto denom = dot(plane.normal, ray.direction);

	if (denom > 1e-6f || denom <-1e-6f)
	{
		const auto t = -(plane.d + (dot(plane.normal, ray.origin)))/denom;
		if (t > 0.0f)
		{
			const auto hitPos = ray.origin + ray.direction * t;
			return HitInfo(true, hitPos, plane.normal, plane.matIndex, t);						
		}
		
	}

	return HitInfo(false, vec3<f32>(), vec3<f32>(), 0, T_MAX);
}

HitInfo raySphereIntersectionTest(const Ray& ray, const Sphere& sphere)
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
		const auto selT = minT > 0.0f ? minT : (maxT > 0.0f ? maxT : 0.0f);

		if (selT > 0.0f)
		{
			const auto hitPos = ray.origin + ray.direction * selT;
			auto normal = normalize(hitPos - sphere.center);

			if (length(ray.origin - sphere.center) < sphere.radius)
			{
				normal = -normal;
			}

			return HitInfo(true, hitPos, normal, sphere.matIndex, selT);
		}				
	}
	
	return HitInfo(false, vec3<f32>(), vec3<f32>(), 0, T_MAX);
}

HitInfo intersectScene(const Ray& ray)
{
	HitInfo closestHitInfo(false, vec3<f32>(), vec3<f32>(), 0, T_MAX);
	
	const auto sphereCount = Scene::get().getSphereCount();
	for (auto i = 0U; i < sphereCount; ++i)
	{
		auto hitInfo = raySphereIntersectionTest(ray, Scene::get().getSphere(i));

		if (hitInfo.hit && hitInfo.t < closestHitInfo.t)
		{			
			closestHitInfo = hitInfo;
		}
	}

	const auto planeCount = Scene::get().getPlaneCount();
	for (auto i = 0U; i < planeCount; ++i)
	{
		auto hitInfo = rayPlaneIntersectionTest(ray, Scene::get().getPlane(i));

		if (hitInfo.hit && hitInfo.t < closestHitInfo.t)
		{			
			closestHitInfo = hitInfo;
		}
	}

	return closestHitInfo;
}

vec3<f32> shade(const Ray& ray, const Light& light, const HitInfo& hitInfo)
{	
	vec3<f32> colorAccum;

	const auto epsilon = 1e-5f;
	const auto displacedHitPos = hitInfo.position + hitInfo.normal * epsilon;

	const auto hitToLight = normalize(light.position - displacedHitPos);	
	const auto viewDir = normalize(displacedHitPos - ray.origin);
	const auto reflDir = normalize(viewDir - hitInfo.normal * dot(viewDir, hitInfo.normal) * 2.0f);
	
	const auto diffuseTerm = max(0.0f, dot(hitInfo.normal, hitToLight));
	const auto& material = Scene::get().getMaterial(hitInfo.surfaceMatIndex);
	const auto specularTerm = powf(max(0.0f, dot(reflDir, hitToLight)), material.glossiness);

	colorAccum += (material.diffuse * light.color) * diffuseTerm;	
	if (light.getLightType() == Light::POINT_LIGHT)
	{				
		colorAccum /= 4 * PI * static_cast<const PointLight&>(light).radius;
	}

	colorAccum += (material.specular * light.color) * specularTerm;	
	

	const auto lightHitInfo = intersectScene(Ray(hitToLight, displacedHitPos));		
	const auto displacedLightHitPos = lightHitInfo.position + lightHitInfo.normal * epsilon;

	// Shadow test
	if (lightHitInfo.hit)
	{																
		auto visibility = 1.0f;

		const auto prevHitToLight = light.position - displacedHitPos;
		const auto revHitToLight = light.position - displacedLightHitPos;
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

	return colorAccum;
}

vec3<f32> traceForEachLight(const Ray& ray, const HitInfo& hitInfo)
{
	if (!hitInfo.hit) return vec3<f32>();

	vec3<f32> fragment = Scene::get().getMaterial(hitInfo.surfaceMatIndex).ambient;

	const auto lightCount = Scene::get().getLightCount();
	for (auto i = 0U; i < lightCount; ++i)
	{
		fragment += shade(ray, Scene::get().getLight(i), hitInfo);
	}	

	return fragment;
}

f32 fresnel(const Ray& ray, const vec3<f32>& normal, const f32 ior)
{
	return powf(1.0f - dot(-ray.direction, normal), Scene::get().getFresnelPower());
}

vec3<f32> trace(const Ray& ray)
{	
	auto initialRay = ray;
	auto initialHitInfo = intersectScene(ray);	

	auto currentRay = initialRay;
	auto currentHitInfo = initialHitInfo;
	auto currentFragColor = traceForEachLight(currentRay, currentHitInfo);
	auto reflectionWeight = 1.0f;

	const auto reflectionCount = Scene::get().getReflectionCount();

	for (auto i = 0U; i < reflectionCount; ++i)
	{
		if (!currentHitInfo.hit) break;

		reflectionWeight *= Scene::get().getMaterial(currentHitInfo.surfaceMatIndex).reflectivity > 0.0f ? 0.5f : 0.0f;
				
		const auto reflectionDir = normalize(ray.direction - currentHitInfo.normal * dot(ray.direction, currentHitInfo.normal) * 2.0f);
		const auto epsilon = 1e-3f;
		auto fresnelKr = 1.0f;
		
		if (Scene::get().getMaterial(currentHitInfo.surfaceMatIndex).refractivity > 1.0f)
		{
			fresnelKr = fresnel(currentRay, currentHitInfo.normal, Scene::get().getMaterial(currentHitInfo.surfaceMatIndex).refractivity);
		}		

		currentRay = Ray(reflectionDir, currentHitInfo.position + epsilon * reflectionDir);
		currentHitInfo = intersectScene(currentRay);		
		currentFragColor += (reflectionWeight * fresnelKr) * traceForEachLight(currentRay, currentHitInfo);
	}

	const auto refractionCount = Scene::get().getRefractionCount();
	auto refractionWeight = 1.0f;
	currentRay = initialRay;	
	currentHitInfo = initialHitInfo;

	for (auto i = 0U; i < refractionCount; ++i)
	{
		if (!currentHitInfo.hit) break;

		refractionWeight *= Scene::get().getMaterial(currentHitInfo.surfaceMatIndex).refractivity > 1.0f ? 0.5f : 0.0f;
		
		
		auto cosi = dot(currentRay.direction, currentHitInfo.normal);						

		auto etaAir = 1.0f;
		auto etaT = Scene::get().getMaterial(currentHitInfo.surfaceMatIndex).refractivity;
		auto n = currentHitInfo.normal;

		if (cosi < 0.0f)
		{
			cosi = -cosi;
		}
		else
		{
			swap(etaAir, etaT);
			n = -currentHitInfo.normal;
		}

		const auto eta = etaAir/etaT;
		const auto k = 1 - eta * eta * (1 - cosi * cosi);
		const auto refractionDir = k < 0.0f ? vec3<f32>() : eta * currentRay.direction + (eta * cosi - sqrtf(k)) * n;
		const auto epsilon = 1e-3f;
		
		auto fresnelKt = 1.0f;

		if (Scene::get().getMaterial(currentHitInfo.surfaceMatIndex).reflectivity > 0.0f)
		{
			fresnelKt = 1.0f - fresnel(currentRay, currentHitInfo.normal, Scene::get().getMaterial(currentHitInfo.surfaceMatIndex).refractivity);
		}

		currentRay = Ray(refractionDir, currentHitInfo.position + epsilon * refractionDir);
		currentHitInfo = intersectScene(currentRay);				
		currentFragColor += (refractionWeight * fresnelKt) * traceForEachLight(currentRay, currentHitInfo);
	}

	return currentFragColor;
}

void render(const sint32 renderWidth,
	        const sint32 renderHeight, 
	        const sint32 targetWidth, 
	        const sint32 targetHeight, 
	        const bool& renderStopFlag,
	        HWND windowHandle,
	        const string& outputFilename)
{
	// Initilize ray tracing result
	Image resultImage(renderWidth, renderHeight);	

	// Calculate ray direction parameters	
	const auto invWidth = 1.0f / renderWidth;
	const auto invHeight = 1.0f / renderHeight;
	const auto fov = PI / 3.0f;
	const auto aspect = static_cast<f32>(renderWidth) / renderHeight;
	const auto angle = tan(fov * 0.5f);

	const auto threadCount = 2;	
	const auto renderStart = chrono::steady_clock::now();

	atomic_long rowsRendered = 0;
	thread workers[threadCount];
	thread announcer([&rowsRendered, &renderStopFlag, renderHeight]()
	{
		auto currentPercent = 0;
		while (rowsRendered != renderHeight)
		{
			if (renderStopFlag) return;

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

		workers[i] = thread([&resultImage, &rowsRendered, &renderStopFlag, i, workPerThread, currentThreadWork, renderWidth, invWidth, invHeight, angle, aspect]()
		{
			for (auto y = i * workPerThread; y < i * workPerThread + currentThreadWork && !renderStopFlag; ++y)
			{
				for (auto x = 0; x < renderWidth; ++x)
				{			
					if (x == 112 && y == 97)
					{
						const auto b = false;
					}

					const auto xx = (2 * ((x + 0.5f) * invWidth) - 1) * angle * aspect;
					const auto yy = (1 - 2 * ((y + 0.5f) * invHeight)) * angle;
					
					vec3<f32> rayDirection(xx, yy, -1.0f);
					rayDirection = normalize(rayDirection);
					Ray ray(rayDirection, vec3<f32>());

					resultImage[y][x] = trace(ray);					
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

	if (renderStopFlag) return;	

	// Scale result
	resultImage.scale((targetWidth + targetHeight) / static_cast<f32>(renderWidth + renderHeight));

	// Create bitmap array
	COLORREF *arr = (COLORREF*)calloc(targetWidth * targetHeight, sizeof(COLORREF));

	for (auto y = 0; y < targetHeight; ++y)
	{
		if (renderStopFlag) return;

		for (auto x = 0; x < targetWidth; ++x)
		{
			if (renderStopFlag) return;

			const auto colorVec = resultImage.getPixel(x, y);
			arr[y * targetWidth + x] = RGB(minf(1.0f, colorVec.z) * 255, minf(1.0f, colorVec.y) * 255, minf(1.0f, colorVec.x) * 255);
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
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE __, LPSTR ___, int ____)
{			
	// Initialize Scene	
	auto renderStopFlag = false;

	// Window Parameters
	const auto windowWidth = 840;
	const auto windowHeight = 680;

	// Constant Render Pamaters
	const auto targetWidth = windowWidth * 4;
	const auto targetHeight = windowHeight * 4;
	const auto initRenderWidth = windowWidth / 8;
	const auto initRenderHeight = windowHeight / 8;

	// Initialize Window
	auto windowHandle = win32::CreateMainWindow(instance, windowWidth, windowHeight, "MinTracer");	

	// Render Target Parameters
	auto renderWidth = initRenderWidth;
	auto renderHeight = initRenderHeight;	
	auto rendering = false;
	auto saveScene = false;
	auto openScene = false;

	MSG msg = {};	
	while (msg.message != WM_QUIT )
	{
		switch(msg.message)
		{
			case WM_PAINT:
			{		
				if (!rendering && renderWidth <= targetWidth)
				{
					rendering = true;
					renderStopFlag = false;

					thread masterRayTraceThread([&rendering, &renderWidth, &renderHeight, windowHeight, windowWidth, targetWidth, targetHeight, &renderStopFlag, windowHandle]()
					{
						render(renderWidth, renderHeight, windowWidth, windowHeight, renderStopFlag, windowHandle, "output_images/last_rendering_" + to_string(renderWidth) + "x" + to_string(renderHeight) + ".bmp");
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
			} break;

			// A menu item has been selected
			// (be it a main or submenu)
			case WM_COMMAND:
			{
				switch (LOWORD(msg.wParam))
				{							
					case win32::GUID_OPEN_SCENE:
					{
						openScene = true;
					} break;

					case win32::GUID_SAVE_SCENE:
					{						
						saveScene = true;
					} break;

					case win32::GUID_QUIT_SCENE: 
					{
						renderStopFlag = true;
						PostQuitMessage(0);
					} break; 

					case win32::GUID_REFL_REFR_COUNT_RENDER:
					{
						win32::CreateReflectionAndRefractionCountDialog(windowHandle, instance);
					} break;

					case win32::GUID_RESTART_RENDER:
					{
						renderStopFlag = true;
						renderWidth = initRenderWidth; 
						renderHeight = initRenderHeight; 
					} break;
				}

				const auto planeCount = Scene::get().getPlaneCount();
				for (auto i = 0U; i < planeCount; ++i)
				{
					if (LOWORD(msg.wParam) == win32::PLANE_GUID_OFFSET + i)
					{
						win32::CreatePlanesEditDialog(windowHandle, instance, i);
					}
				}

				const auto sphereCount = Scene::get().getSphereCount();
				for (auto i = 0U; i < sphereCount; ++i)
				{
					if (LOWORD(msg.wParam) == win32::SPHERE_GUID_OFFSET + i)
					{
						win32::CreateSpheresEditDialog(windowHandle, instance, i);
					} 
				}

				const auto lightCount = Scene::get().getLightCount();
				for (auto i = 0U; i < lightCount; ++i)
				{
					if (LOWORD(msg.wParam) == win32::LIGHT_GUID_OFFSET + i)
					{
						win32::CreateLightsEditDialog(windowHandle, instance, i);
					}
				}

			} break;

			// A trackbar from children dialogs was updated
			// and hence rendering needs to be restarted
			case WM_HSCROLL:
			{				
				renderStopFlag = true;
				renderWidth = initRenderWidth;
				renderHeight = initRenderHeight;
			} break;
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

		if (saveScene)
		{
			win32::CreateIODialog(windowHandle, instance, win32::IO_DIALOG_TYPE::SAVE_AS, [](const win32::IO_DIALOG_RESULT_TYPE resultType)
			{
				switch (resultType)
				{
					case win32::SUCCESS:
					{
						MessageBox(NULL, "Scene successfully saved", "Save Scene", MB_OK);
					} break;

					case win32::FAILURE:
					{
						MessageBox(NULL, "Error: Could not save scene", "Error", MB_OK | MB_ICONERROR);
					} break;
				}
				
			});
			saveScene = false;
		}

		if (openScene)
		{
			win32::CreateIODialog(windowHandle, instance, win32::IO_DIALOG_TYPE::OPEN, nullptr);
			openScene = false;
		}
	}		

	renderStopFlag = true;

	// Wait for threads to catch up to the rendering stop flag and shut down
	Sleep(1000);

	return 0;
}