/**********************************************************************/
/** main.cpp by Alex Koukoulas (C) 2017 All Rights Reserved          **/
/** File Description:                                                **/
/**********************************************************************/

#if defined(DEBUG) || defined(_DEBUG)
#include <vld.h>
#endif

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <Windows.h>
#include <CommCtrl.h>
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
#include "string_utils.h"

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

void render(const Scene& scene,
	        const sint32 renderWidth,
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
				//OutputDebugString(string("Ray Tracing " + to_string(currentPercent) + "% complete\n").c_str());
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

		workers[i] = thread([&scene, &resultImage, &rowsRendered, &renderStopFlag, i, workPerThread, currentThreadWork, renderWidth, invWidth, invHeight, angle, aspect]()
		{
			for (auto y = i * workPerThread; y < i * workPerThread + currentThreadWork && !renderStopFlag; ++y)
			{
				for (auto x = 0; x < renderWidth; ++x)
				{
					if (renderStopFlag) return;

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

static const uint32 IDM_QUIT_SCENE = 11;
static const uint32 IDM_LIGHTS_EDIT= 21;
static const uint32 IDM_OBJECTS_EDIT = 22;
static const uint32 IDM_RESTART_RENDER = 31;

void addMenus(HWND hwnd)
{
	HMENU hMenubar = CreateMenu();
	HMENU hSceneMenu = CreateMenu();
	HMENU hEditMenu = CreateMenu();
	HMENU hRenderMenu = CreateMenu();	
	
	// Scene Menu	
	AppendMenuW(hSceneMenu, MF_STRING, IDM_QUIT_SCENE, L"&Quit");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hSceneMenu, L"&Scene");

	// Edit Menu
	AppendMenuW(hEditMenu, MF_STRING, IDM_LIGHTS_EDIT, L"&Light");
	AppendMenuW(hEditMenu, MF_STRING, IDM_OBJECTS_EDIT, L"&Objects");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hEditMenu, L"&Edit");

	// Render Menu
	AppendMenuW(hRenderMenu, MF_STRING, IDM_RESTART_RENDER, L"&Restart Rendering");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hRenderMenu, L"&Render");

	// Master Menu Bar
	SetMenu(hwnd, hMenubar);	
}

LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
		{
			addMenus(hwnd);
		} break;		

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

unique_ptr<Scene> createScene()
{
	// Initialize scene
	unique_ptr<Scene> scene = make_unique<Scene>();

	scene->lights.emplace_back(vec3<f32>(0.0f, 0.0f, 0.0f), vec3<f32>(0.5f, 0.5f, 0.5f));

	scene->materials.emplace_back(vec3<f32>(0.0f, 0.0f, 0.0f), vec3<f32>(0.0f, 0.0f, 0.0f), vec3<f32>(0.0f, 0.0f, 0.0f), 0.0f);
	scene->materials.emplace_back(vec3<f32>(0.3f, 0.1f, 0.1f), vec3<f32>(0.9f, 0.3f, 0.3f), vec3<f32>(0.9f, 0.3f, 0.3f), 128.0f);
	scene->materials.emplace_back(vec3<f32>(0.1f, 0.2f, 0.4f), vec3<f32>(0.3f, 0.5f, 0.9f), vec3<f32>(0.3f, 0.5f, 0.9f), 64.0f);
	scene->materials.emplace_back(vec3<f32>(0.2f, 0.2f, 0.2f), vec3<f32>(0.5f, 0.5f, 0.5f), vec3<f32>(0.5f, 0.5f, 0.5f), 1.0f);
	scene->materials.emplace_back(vec3<f32>(0.1f, 0.1f, 0.4f), vec3<f32>(0.3f, 0.3f, 0.9f), vec3<f32>(0.3f, 0.3f, 0.9f), 24.0f);

	scene->spheres.emplace_back(2.0f, vec3<f32>(-3.0f, 1.0f, -9.0f), 1);
	scene->spheres.emplace_back(1.7f, vec3<f32>(2.3f, 0.0f, -9.0f), 2);
	scene->spheres.emplace_back(0.5f, vec3<f32>(0.0f, 0.0f, -5.0f), 4);

	scene->planes.emplace_back(vec3<f32>(0.0f, 0.0f, 1.0f), 10.0f, 3);
	scene->planes.emplace_back(vec3<f32>(0.0f, 1.0f, 0.0f), 2.0f, 3);
	scene->planes.emplace_back(vec3<f32>(0.0f, -1.0f, 0.0f), 4.0f, 3);
	scene->planes.emplace_back(vec3<f32>(-1.0f, 0.0f, 0.0f), 4.0f, 3);
	scene->planes.emplace_back(vec3<f32>(1.0f, 0.0f, 0.0f), 4.0f, 3);
	
	return scene;
}

HWND WINAPI CreateTrackbar(
	HWND hwndDlg,        // handle of dialog box (parent window) 
	HINSTANCE hInstance, // Instance	
	const string& title,
	const sint32 x, 
	const sint32 y,
	const uint32 pos)	
{	
	auto hwndTrack = CreateWindow(TRACKBAR_CLASS,
		                          title.c_str(),
		                          WS_CHILD | WS_VISIBLE | TBS_ENABLESELRANGE | TBS_NOTICKS,
		                          x, y,
		                          200, 30,
		                          hwndDlg,
		                          (HMENU)1002, 
		                          hInstance,
		                          NULL);

	SendMessage(hwndTrack, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 100)); 
	SendMessage(hwndTrack, TBM_SETPAGESIZE, 0, (LPARAM)4);                 
	SendMessage(hwndTrack, TBM_SETSEL, (WPARAM)FALSE, (LPARAM)0);
	SendMessage(hwndTrack, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)pos);
	
	return hwndTrack;
}

static HWND lightPositionX, lightPositionY, lightPositionZ;
static HWND lightColorX, lightColorY, lightColorZ;

LRESULT CALLBACK lightEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	static const uint32 BUTTON_ID = 10;

	switch (msg)
	{		
	    case WM_CREATE: 
		{
			// Light Offset Labels			
			CreateWindow("STATIC", "Light Position", WS_VISIBLE | WS_CHILD | SS_LEFT, 130, 10, 100, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "-10.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 36, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 36, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "+10.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 225, 36, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "-10.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 86, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 86, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "+10.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 225, 86, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "-10.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 136, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 136, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "+10.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 225, 136, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "x", WS_VISIBLE | WS_CHILD | SS_LEFT, 60, 50,  80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "y", WS_VISIBLE | WS_CHILD | SS_LEFT, 60, 100, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "z", WS_VISIBLE | WS_CHILD | SS_LEFT, 60, 150, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

			// Light Color Labels
			CreateWindow("STATIC", "Light Color", WS_VISIBLE | WS_CHILD | SS_LEFT, 130, 210, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 236, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.5", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 236, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "1.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 240, 236, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 286, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.5", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 286, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "1.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 240, 286, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 336, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.5", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 336, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "1.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 240, 336, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "r", WS_VISIBLE | WS_CHILD | SS_LEFT, 60, 250, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "g", WS_VISIBLE | WS_CHILD | SS_LEFT, 60, 300, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "b", WS_VISIBLE | WS_CHILD | SS_LEFT, 60, 350, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

			// Confirmation button
			CreateWindow("BUTTON", "OK", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 116, 400, 100, 30, hwnd, (HMENU)BUTTON_ID, GetModuleHandle(NULL), NULL);

			// Light Position Trackbars
			lightPositionX = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Light x", 70, 50, 50);
			lightPositionY = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Light y", 70, 100, 50);
			lightPositionZ = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Light z", 70, 150, 50);

			// Light Color Trackbars
			lightColorX = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Color x", 70, 250, 50);
			lightColorY = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Color y", 70, 300, 50);
			lightColorZ = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Color z", 70, 350, 50);
		} break;

		case WM_CTLCOLORSTATIC:
		{
			HDC hdcStatic = (HDC)wParam;			
			SetBkColor(hdcStatic, RGB(255, 255, 255));
			return (INT_PTR)CreateSolidBrush(RGB(255, 255, 255));
		} break;

		case WM_COMMAND:
		{
			if (LOWORD(wParam) == BUTTON_ID)
			{
				DestroyWindow(hwnd);
			}
		} break;

		case WM_HSCROLL: 
		{		
			// Propagate message to parent
			const auto lo = LOWORD(wParam);

			if (lo == SB_THUMBTRACK || lo == SB_THUMBPOSITION)
			{
				PostMessage(GetParent(hwnd), WM_HSCROLL, wParam, lParam);
			}					
		} break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND WINAPI createLightEditDialog(HWND hwnd, HINSTANCE hInstance)
{
	
	WNDCLASS wc = {};
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = lightEditWndProc;
	wc.lpszClassName = "LightEdit";
	wc.lpszMenuName = 0;
	wc.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClass(&wc);
	

	const auto width = 330;
	const auto height = 490;
	const auto x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	const auto y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

	auto hwndLightEdit = CreateWindow(
		"LightEdit",
		"Edit Lights",
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
		x, y,
		width, height,
		hwnd,
		NULL,
		hInstance,
		NULL
	);		

	SetFocus(hwndLightEdit);
	ShowWindow(hwndLightEdit, SW_SHOW);
	return hwndLightEdit;
}

static HWND sphereOffsetX, sphereOffsetY, sphereOffsetZ;
static HWND sphereRadius;
static HWND sphereGlossiness;

LRESULT CALLBACK objectsEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static const uint32 BUTTON_ID = 10;

	switch (msg)
	{
	case WM_CREATE:
	{
		// Sphere Position Labels			
		CreateWindow("STATIC", "Sphere Position", WS_VISIBLE | WS_CHILD | SS_LEFT, 115, 10, 120, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "-10.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 36, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 36, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "+10.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 225, 36, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "-10.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 86, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 86, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "+10.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 225, 86, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "-10.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 136, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 136, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "+10.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 225, 136, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "x", WS_VISIBLE | WS_CHILD | SS_LEFT, 60, 50, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "y", WS_VISIBLE | WS_CHILD | SS_LEFT, 60, 100, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "z", WS_VISIBLE | WS_CHILD | SS_LEFT, 60, 150, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

		// Sphere Radius Labels
		CreateWindow("STATIC", "Sphere Radius", WS_VISIBLE | WS_CHILD | SS_LEFT, 115, 210, 100, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 236, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "5.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 236, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "10.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 232, 236, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "Rad", WS_VISIBLE | WS_CHILD | SS_LEFT, 45, 250, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

		// Sphere Glossiness
		CreateWindow("STATIC", "Sphere Glossiness", WS_VISIBLE | WS_CHILD | SS_LEFT, 105, 300, 130, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 325, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "128.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 155, 325, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "256.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 226, 325, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
		CreateWindow("STATIC", "Glos", WS_VISIBLE | WS_CHILD | SS_LEFT, 40, 340, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

		// Confirmation button
		CreateWindow("BUTTON", "OK", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 116, 400, 100, 30, hwnd, (HMENU)BUTTON_ID, GetModuleHandle(NULL), NULL);

		// Sphere Position Trackbars
		sphereOffsetX = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Sphere x", 70, 50, 35);
		sphereOffsetY = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Sphere y", 70, 100, 45);
		sphereOffsetZ = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Sphere z", 70, 150, 5);
		
		// Sphere Radius Trackbar
		sphereRadius = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Sphere Radius", 70, 250, 20);

		// Sphere Glossiness Trackbar
		sphereGlossiness = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Sphere Glossiness", 70, 340, 50);

		/*
		lightColorX = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Color x", 70, 250);
		lightColorY = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Color y", 70, 300);
		lightColorZ = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Color z", 70, 350);*/
	} break;

	case WM_CTLCOLORSTATIC:
	{
		HDC hdcStatic = (HDC)wParam;
		SetBkColor(hdcStatic, RGB(255, 255, 255));
		return (INT_PTR)CreateSolidBrush(RGB(255, 255, 255));
	} break;

	case WM_COMMAND:
	{
		if (LOWORD(wParam) == BUTTON_ID)
		{
			DestroyWindow(hwnd);
		}
	} break;

	case WM_HSCROLL:
	{
		// Propagate message to parent
		const auto lo = LOWORD(wParam);

		if (lo == SB_THUMBTRACK || lo == SB_THUMBPOSITION)
		{
			PostMessage(GetParent(hwnd), WM_HSCROLL, wParam, lParam);
		}
	} break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND WINAPI createObjectsEditDialog(HWND hwnd, HINSTANCE hInstance)
{

	WNDCLASS wc = {};
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = objectsEditWndProc;
	wc.lpszClassName = "ObjectsEdit";
	wc.lpszMenuName = 0;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&wc);


	const auto width = 330;
	const auto height = 490;
	const auto x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	const auto y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

	auto hwndLightEdit = CreateWindow(
		"ObjectsEdit",
		"Edit Objects",
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
		x, y,
		width, height,
		hwnd,
		NULL,
		hInstance,
		NULL
	);

	SetFocus(hwndLightEdit);
	ShowWindow(hwndLightEdit, SW_SHOW);
	return hwndLightEdit;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE __, LPSTR ___, int ____)
{			
	// Initialize Scene
	auto scene = createScene();
	auto renderStopFlag = false;

	// Window, Render & Target Parameters
	const auto windowWidth = 840;
	const auto windowHeight = 680;
	const auto targetWidth = windowWidth * 4;
	const auto targetHeight = windowHeight * 4;
	const auto initRenderWidth = windowWidth / 8;
	const auto initRenderHeight = windowHeight / 8;
	auto renderWidth = initRenderWidth;
	auto renderHeight = initRenderHeight;

	auto windowHandle = createWindow(instance, windowWidth, windowHeight, "MinTracer");	
	
	auto rendering = false;

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

					thread masterRayTraceThread([&scene, &rendering, &renderWidth, &renderHeight, windowHeight, windowWidth, targetWidth, targetHeight, &renderStopFlag, windowHandle]()
					{
						render(*scene, renderWidth, renderHeight, windowWidth, windowHeight, renderStopFlag, windowHandle, "output_images/last_rendering_" + to_string(renderWidth) + "x" + to_string(renderHeight) + ".bmp");
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

			case WM_COMMAND:
			{
				switch (LOWORD(msg.wParam))
				{					
					case IDM_LIGHTS_EDIT: 
					{
						createLightEditDialog(windowHandle, instance);
					} break;

					case IDM_OBJECTS_EDIT: 
					{				
						createObjectsEditDialog(windowHandle, instance);
					} break;

					case IDM_QUIT_SCENE: 
					{
						renderStopFlag = true;
						PostQuitMessage(0);
					} break; 

					case IDM_RESTART_RENDER:
					{
						renderStopFlag = true;
						renderWidth = initRenderWidth; 
						renderHeight = initRenderHeight; 
					} break;
				}
			} break;

			case WM_HSCROLL:
			{
				const auto lo = LOWORD(msg.wParam);
				const auto hi = HIWORD(msg.wParam);

				if (msg.lParam == (LPARAM)lightPositionX)
				{
					scene->lights[0].position.x = (hi - 50) / 10.0f;
				}
				else if (msg.lParam == (LPARAM)lightPositionY)
				{
					scene->lights[0].position.y = (hi - 50) / 10.0f;
				}
				else if (msg.lParam == (LPARAM)lightPositionZ)
				{
					scene->lights[0].position.z = (hi - 50) / 10.0f;
				}		
				else if (msg.lParam == (LPARAM)lightColorX)
				{
					scene->lights[0].color.x = hi/100.0f;
				}
				else if (msg.lParam == (LPARAM)lightColorY)
				{
					scene->lights[0].color.y = hi/100.0f;
				}
				else if (msg.lParam == (LPARAM)lightColorZ)
				{
					scene->lights[0].color.z = hi/100.0f;
				}
				else if (msg.lParam == (LPARAM)sphereOffsetX)
				{
					scene->spheres[0].center.x = (hi - 50)/5.0f;
				}
				else if (msg.lParam == (LPARAM)sphereOffsetY)
				{
					scene->spheres[0].center.y = (hi - 50)/5.0f;
				}
				else if (msg.lParam == (LPARAM)sphereOffsetZ)
				{
					scene->spheres[0].center.z = (hi - 50)/5.0f;
				}
				else if (msg.lParam == (LPARAM)sphereRadius)
				{
					scene->spheres[0].radius = hi/10.0f;
				}
				else if (msg.lParam == (LPARAM)sphereGlossiness)
				{
					scene->materials[1].glossiness = hi * 2.56f;
				}

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
	}		

	renderStopFlag = true;

	// Wait for threads to catch up to the rendering stop flag and shut down
	Sleep(1000);

	return 0;
}