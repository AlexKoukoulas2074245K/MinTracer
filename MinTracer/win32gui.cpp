/**********************************************************************/
/** win32gui.cpp by Alex Koukoulas (C) 2017 All Rights Reserved      **/
/** File Description:                                                **/
/**********************************************************************/

#pragma once

// Local Headers
#include "win32gui.h"
#include "scene.h"

// Remote Headers

// Use Modern GUI style
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

static uint32 currentLightIndex = 0;
static uint32 currentSphereIndex = 0;
static uint32 currentPlaneIndex = 0;

static HWND WINAPI CreateTrackbar(
	HWND hwndDlg,        // handle of dialog box (parent window) 
	HINSTANCE hInstance, // Instance	
	const std::string& title,
	const sint32 x,
	const sint32 y,
	const uint32 pos,
	const uint32 steps = 100)
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

	SendMessage(hwndTrack, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, steps));
	SendMessage(hwndTrack, TBM_SETPAGESIZE, 0, (LPARAM)4);
	SendMessage(hwndTrack, TBM_SETSEL, (WPARAM)FALSE, (LPARAM)0);
	SendMessage(hwndTrack, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)pos);

	return hwndTrack;
}

void CreateMenus(HWND hwnd)
{
	HMENU hMenubar = CreateMenu();
	HMENU hSceneMenu = CreateMenu();
	HMENU hEditMenu = CreateMenu();
	HMENU hRenderMenu = CreateMenu();
	HMENU hLightsSubMenu = CreatePopupMenu();
	HMENU hSpheresSubMenu = CreatePopupMenu();
	HMENU hPlanesSubMenu = CreatePopupMenu();

	// Scene Menu	
	AppendMenuW(hSceneMenu, MF_STRING, win32::GUID_OPEN_SCENE, L"&Open");
	AppendMenuW(hSceneMenu, MF_STRING, win32::GUID_SAVE_SCENE, L"&Save As..");
	AppendMenuW(hSceneMenu, MF_STRING, win32::GUID_QUIT_SCENE, L"&Quit");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hSceneMenu, L"&Scene");

	// Lights Submenu
	AppendMenuW(hEditMenu, MF_POPUP | MF_STRING, (UINT_PTR)hLightsSubMenu, L"&Lights");
	const auto lightCount = Scene::get().getLightCount();
	for (auto i = 0U; i < lightCount; ++i)
	{
		std::wstring lightEntryName = (Scene::get().getLight(i).getLightType() == Light::DIR_LIGHT ? L"&Light " : L"&PointLight ") + std::to_wstring(i);
		AppendMenuW(hLightsSubMenu, MF_STRING, win32::LIGHT_GUID_OFFSET + i, lightEntryName.c_str());
	}

	// Spheres Submenu
	AppendMenuW(hEditMenu, MF_POPUP | MF_STRING, (UINT_PTR)hSpheresSubMenu, L"&Spheres");			
	const auto sphereCount = Scene::get().getSphereCount();
	for (auto i = 0U; i < sphereCount; ++i)
	{
		std::wstring sphereEntryName = L"&Sphere " + std::to_wstring(i);
		AppendMenuW(hSpheresSubMenu, MF_STRING, win32::SPHERE_GUID_OFFSET + i, sphereEntryName.c_str());
	}	

	// Plane Submenu
	AppendMenuW(hEditMenu, MF_POPUP | MF_STRING, (UINT_PTR)hPlanesSubMenu, L"&Planes");
	const auto planeCount = Scene::get().getPlaneCount();
	for (auto i = 0U; i < planeCount; ++i)
	{
		std::wstring planeEntryName = L"&Plane " + std::to_wstring(i);
		AppendMenuW(hPlanesSubMenu, MF_STRING, win32::PLANE_GUID_OFFSET + i, planeEntryName.c_str());
	}

	// Edit Menu
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hEditMenu, L"&Edit");

	// Render Menu
	AppendMenuW(hRenderMenu, MF_STRING, win32::GUID_REFL_REFR_COUNT_RENDER, L"&Reflection && Refraction");
	AppendMenuW(hRenderMenu, MF_STRING, win32::GUID_RESTART_RENDER, L"&Restart Rendering");	
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hRenderMenu, L"&Render");

	// Master Menu Bar
	SetMenu(hwnd, hMenubar);
}

LRESULT CALLBACK PlanesEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static const uint32 BUTTON_ID = 10;

	static HWND planeNormalXTrackbar;
	static HWND planeNormalYTrackbar;
	static HWND planeNormalZTrackbar;
	static HWND planeDistanceTrackbar;
	static HWND planeGlossinessTrackbar;
	static HWND planeReflectivityTrackbar;
	static HWND planeRefractivityTrackbar;

	switch (msg)
	{
		case WM_CREATE:
		{
			// Plane Normal Labels			
			CreateWindow("STATIC", "Plane Normal", WS_VISIBLE | WS_CHILD | SS_LEFT, 115, 10, 120, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "-1.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 36, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 36, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "+1.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 235, 36, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "-1.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 86, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 86, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "+1.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 235, 86, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "-1.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 136, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 136, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "+1.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 235, 136, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "x", WS_VISIBLE | WS_CHILD | SS_LEFT, 60, 50, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "y", WS_VISIBLE | WS_CHILD | SS_LEFT, 60, 100, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "z", WS_VISIBLE | WS_CHILD | SS_LEFT, 60, 150, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

			// Plane Distance Labels
			CreateWindow("STATIC", "Plane Distance", WS_VISIBLE | WS_CHILD | SS_LEFT, 115, 210, 100, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "-25.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 236, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 236, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "25.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 232, 236, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "d", WS_VISIBLE | WS_CHILD | SS_LEFT, 45, 250, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

			// Plane Glossiness Labels
			CreateWindow("STATIC", "Plane Glossiness", WS_VISIBLE | WS_CHILD | SS_LEFT, 105, 300, 130, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 325, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "128.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 155, 325, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "256.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 226, 325, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "Glos", WS_VISIBLE | WS_CHILD | SS_LEFT, 40, 340, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

			// Sphere Reflectivity
			CreateWindow("STATIC", "Plane Reflectivity", WS_VISIBLE | WS_CHILD | SS_LEFT, 105, 390, 130, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 414, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.5", WS_VISIBLE | WS_CHILD | SS_LEFT, 158, 414, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "1.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 240, 414, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "Refl", WS_VISIBLE | WS_CHILD | SS_LEFT, 40, 430, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

			// Sphere Reflectivity
			CreateWindow("STATIC", "Plane Refractivity", WS_VISIBLE | WS_CHILD | SS_LEFT, 105, 480, 130, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 503, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "1.5", WS_VISIBLE | WS_CHILD | SS_LEFT, 158, 503, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "3.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 240, 503, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "Refr", WS_VISIBLE | WS_CHILD | SS_LEFT, 40, 520, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

			// Confirmation button
			CreateWindow("BUTTON", "OK", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 116, 570, 100, 30, hwnd, (HMENU)BUTTON_ID, GetModuleHandle(NULL), NULL);

			// Plane Position Trackbars
			planeNormalXTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Plane Normal x", 70, 50, static_cast<uint32>(Scene::get().getPlane(currentPlaneIndex).normal.x * 50.0f + 50));
			planeNormalYTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Plane Normal y", 70, 100, static_cast<uint32>(Scene::get().getPlane(currentPlaneIndex).normal.y * 50.0f + 50));
			planeNormalZTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Plane Normal z", 70, 150, static_cast<uint32>(Scene::get().getPlane(currentPlaneIndex).normal.z * 50.0f + 50));

			// Plane Distance Trackbar
			planeDistanceTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Plane Distance", 70, 250, static_cast<uint32>(Scene::get().getPlane(currentPlaneIndex).d * 2.0f + 50));

			// Plane Glossiness Trackbar
			planeGlossinessTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Plane Glossiness", 70, 340, static_cast<uint32>(Scene::get().getMaterial(Scene::get().getPlane(currentPlaneIndex).matIndex).glossiness / 2.56f));

			// Plane Reflectivity Trackbar
			planeReflectivityTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Plane Reflectivity", 70, 430, static_cast<uint32>(Scene::get().getMaterial(Scene::get().getPlane(currentPlaneIndex).matIndex).reflectivity * 100.0f));

			// Plane Refractivity Trackbar
			planeRefractivityTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Plane Refractivity", 70, 520, static_cast<uint32>(Scene::get().getMaterial(Scene::get().getPlane(currentPlaneIndex).matIndex).refractivity * 33.0f));
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
			const auto hi = HIWORD(wParam);

			if (lo == SB_THUMBTRACK || lo == SB_THUMBPOSITION)
			{
				if (lParam == (LPARAM)planeNormalXTrackbar)
				{
					Scene::get().getPlane(currentPlaneIndex).normal.x = (hi - 50) / 50.0f;
					Scene::get().getPlane(currentPlaneIndex).normal = normalize(Scene::get().getPlane(currentPlaneIndex).normal);
				}
				else if (lParam == (LPARAM)planeNormalYTrackbar)
				{
					Scene::get().getPlane(currentPlaneIndex).normal.y = (hi - 50) / 50.0f;
					Scene::get().getPlane(currentPlaneIndex).normal = normalize(Scene::get().getPlane(currentPlaneIndex).normal);
				}
				else if (lParam == (LPARAM)planeNormalZTrackbar)
				{
					Scene::get().getPlane(currentPlaneIndex).normal.z = (hi - 50) / 50.0f;
					Scene::get().getPlane(currentPlaneIndex).normal = normalize(Scene::get().getPlane(currentPlaneIndex).normal);
				}
				else if (lParam == (LPARAM)planeDistanceTrackbar)
				{
					Scene::get().getPlane(currentPlaneIndex).d = (hi - 50)/ 2.0f;
				}
				else if (lParam == (LPARAM)planeGlossinessTrackbar)
				{
					Scene::get().getMaterial(Scene::get().getPlane(currentPlaneIndex).matIndex).glossiness = hi * 2.56f;
				}
				else if (lParam == (LPARAM)planeReflectivityTrackbar)
				{
					Scene::get().getMaterial(Scene::get().getPlane(currentPlaneIndex).matIndex).reflectivity = hi / 100.0f;
				}
				else if (lParam == (LPARAM)planeRefractivityTrackbar)
				{
					Scene::get().getMaterial(Scene::get().getPlane(currentPlaneIndex).matIndex).refractivity = hi / 33.0f;
				}

				PostMessage(GetParent(hwnd), WM_HSCROLL, wParam, lParam);
			}
		} break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK SpheresEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static const uint32 BUTTON_ID = 10;

	static HWND sphereOffsetXTrackbar;
	static HWND sphereOffsetYTrackbar;
	static HWND sphereOffsetZTrackbar;
	static HWND sphereRadiusTrackbar;
	static HWND sphereGlossinessTrackbar;	
	static HWND sphereReflectivityTrackbar;
	static HWND sphereRefractivityTrackbar;

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

			// Sphere Reflectivity
			CreateWindow("STATIC", "Sphere Reflectivity", WS_VISIBLE | WS_CHILD | SS_LEFT, 105, 390, 130, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 414, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.5", WS_VISIBLE | WS_CHILD | SS_LEFT, 158, 414, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "1.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 240, 414, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "Refl", WS_VISIBLE | WS_CHILD | SS_LEFT, 40, 430, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

			// Sphere Reflectivity
			CreateWindow("STATIC", "Sphere Refractivity", WS_VISIBLE | WS_CHILD | SS_LEFT, 105, 480, 130, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 503, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "1.5", WS_VISIBLE | WS_CHILD | SS_LEFT, 158, 503, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "3.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 240, 503, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "Refr", WS_VISIBLE | WS_CHILD | SS_LEFT, 40, 520, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

			// Confirmation button
			CreateWindow("BUTTON", "OK", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 116, 570, 100, 30, hwnd, (HMENU)BUTTON_ID, GetModuleHandle(NULL), NULL);

			// Sphere Position Trackbars
			sphereOffsetXTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Sphere x", 70, 50, static_cast<uint32>(Scene::get().getSphere(currentSphereIndex).center.x * 5.0f + 50));
			sphereOffsetYTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Sphere y", 70, 100, static_cast<uint32>(Scene::get().getSphere(currentSphereIndex).center.y * 5.0f + 50));
			sphereOffsetZTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Sphere z", 70, 150, static_cast<uint32>(Scene::get().getSphere(currentSphereIndex).center.z * 5.0f + 50));

			// Sphere Radius Trackbar
			sphereRadiusTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Sphere Radius", 70, 250, static_cast<uint32>(Scene::get().getSphere(currentSphereIndex).radius * 10.0f));

			// Sphere Glossiness Trackbar
			sphereGlossinessTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Sphere Glossiness", 70, 340, static_cast<uint32>(Scene::get().getMaterial(Scene::get().getSphere(currentSphereIndex).matIndex).glossiness / 2.56f));

			// Sphere Reflectivity Trackbar
			sphereReflectivityTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Sphere Reflectivity", 70, 430, static_cast<uint32>(Scene::get().getMaterial(Scene::get().getSphere(currentSphereIndex).matIndex).reflectivity * 100.0f));

			// Sphere Refractivity Trackbar
			sphereRefractivityTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Sphere Refractivity", 70, 520, static_cast<uint32>(Scene::get().getMaterial(Scene::get().getSphere(currentSphereIndex).matIndex).refractivity * 33.0f));

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
			const auto hi = HIWORD(wParam);

			if (lo == SB_THUMBTRACK || lo == SB_THUMBPOSITION)
			{				
                if (lParam == (LPARAM)sphereOffsetXTrackbar)
				{
					Scene::get().getSphere(currentSphereIndex).center.x = (hi - 50) / 5.0f;
				}
				else if (lParam == (LPARAM)sphereOffsetYTrackbar)
				{
					Scene::get().getSphere(currentSphereIndex).center.y = (hi - 50) / 5.0f;
				}
				else if (lParam == (LPARAM)sphereOffsetZTrackbar)
				{
					Scene::get().getSphere(currentSphereIndex).center.z = (hi - 50) / 5.0f;
				}
				else if (lParam == (LPARAM)sphereRadiusTrackbar)
				{
					Scene::get().getSphere(currentSphereIndex).radius = hi / 10.0f;
				}
				else if (lParam == (LPARAM)sphereGlossinessTrackbar)
				{
					Scene::get().getMaterial(Scene::get().getSphere(currentSphereIndex).matIndex).glossiness = hi * 2.56f;
				}				
				else if (lParam == (LPARAM)sphereReflectivityTrackbar)
				{
					Scene::get().getMaterial(Scene::get().getSphere(currentSphereIndex).matIndex).reflectivity = hi / 100.0f;
				}
				else if (lParam == (LPARAM)sphereRefractivityTrackbar)
				{
					Scene::get().getMaterial(Scene::get().getSphere(currentSphereIndex).matIndex).refractivity = hi / 33.0f;
				}

				PostMessage(GetParent(hwnd), WM_HSCROLL, wParam, lParam);
			}
		} break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK LightEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static const uint32 BUTTON_ID = 10;

	static HWND lightPositionXTrackbar;
	static HWND lightPositionYTrackbar;
	static HWND lightPositionZTrackbar;
	static HWND lightColorXTrackbar;
	static HWND lightColorYTrackbar;
	static HWND lightColorZTrackbar;
	static HWND pointLightRadiusTrackbar;

	switch (msg)
	{
		case WM_CREATE:
		{
			// Light Offset Labels			
			CreateWindow("STATIC", "Light Position", WS_VISIBLE | WS_CHILD | SS_LEFT, 130, 10, 100, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "-20.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 36, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 36, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "+20.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 225, 36, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "-20.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 86, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 86, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "+20.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 225, 86, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "-20.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 136, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 136, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "+20.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 225, 136, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "x", WS_VISIBLE | WS_CHILD | SS_LEFT, 60, 50, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
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
			CreateWindow("BUTTON", "OK", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 116, 480, 100, 30, hwnd, (HMENU)BUTTON_ID, GetModuleHandle(NULL), NULL);

			// Light Position Trackbars
			lightPositionXTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Light x", 70, 50, static_cast<uint32>(Scene::get().getLight(currentLightIndex).position.x * 2.5f + 50));
			lightPositionYTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Light y", 70, 100, static_cast<uint32>(Scene::get().getLight(currentLightIndex).position.y * 2.5f + 50));
			lightPositionZTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Light z", 70, 150, static_cast<uint32>(Scene::get().getLight(currentLightIndex).position.z * 2.5f + 50));

			// Light Color Trackbars
			lightColorXTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Color x", 70, 250, static_cast<uint32>(Scene::get().getLight(currentLightIndex).color.x * 100.0f));
			lightColorYTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Color y", 70, 300, static_cast<uint32>(Scene::get().getLight(currentLightIndex).color.y * 100.0f));
			lightColorZTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Color z", 70, 350, static_cast<uint32>(Scene::get().getLight(currentLightIndex).color.z * 100.0f));

			// Point Light Radius
			if (Scene::get().getLight(currentLightIndex).getLightType() == Light::POINT_LIGHT)
			{
				// Sphere Radius Labels
				CreateWindow("STATIC", "Point Light Radius", WS_VISIBLE | WS_CHILD | SS_LEFT, 115, 400, 140, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
				CreateWindow("STATIC", "0.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 422, 40, 24, hwnd, NULL, GetModuleHandle(NULL), NULL);
				CreateWindow("STATIC", "5.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 161, 422, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
				CreateWindow("STATIC", "10.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 232, 422, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
				CreateWindow("STATIC", "Rad", WS_VISIBLE | WS_CHILD | SS_LEFT, 45, 435, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

				PointLight& pl = static_cast<PointLight&>(Scene::get().getLight(currentLightIndex));
				pointLightRadiusTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Point Light Radius", 70, 440, static_cast<uint32>(pl.radius * 10.0f));
			}			

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
			const auto hi = HIWORD(wParam);


			if (lo == SB_THUMBTRACK || lo == SB_THUMBPOSITION)
			{
				if (lParam == (LPARAM)lightPositionXTrackbar)
				{
					Scene::get().getLight(currentLightIndex).position.x = (hi - 50) / 2.5f;
				}
				else if (lParam == (LPARAM)lightPositionYTrackbar)
				{
					Scene::get().getLight(currentLightIndex).position.y = (hi - 50) / 2.5f;
				}
				else if (lParam == (LPARAM)lightPositionZTrackbar)
				{
					Scene::get().getLight(currentLightIndex).position.z = (hi - 50) / 2.5f;
				}
				else if (lParam == (LPARAM)lightColorXTrackbar)
				{
					Scene::get().getLight(currentLightIndex).color.x = hi / 100.0f;
				}
				else if (lParam == (LPARAM)lightColorYTrackbar)
				{
					Scene::get().getLight(currentLightIndex).color.y = hi / 100.0f;
				}
				else if (lParam == (LPARAM)lightColorZTrackbar)
				{
					Scene::get().getLight(currentLightIndex).color.z = hi / 100.0f;
				}				
				else if (lParam == (LPARAM)pointLightRadiusTrackbar)
				{
					static_cast<PointLight&>(Scene::get().getLight(currentLightIndex)).radius = hi / 10.0f;
				}

				PostMessage(GetParent(hwnd), WM_HSCROLL, wParam, lParam);
			}
		} break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK ReflectionCountWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static const uint32 BUTTON_ID = 10;
	static HWND reflectionCountTrackbar;
	static HWND refractionCountTrackbar;
	static HWND fresnelPowerTrackbar;

	switch (msg)
	{
		case WM_CREATE:
		{
			// Reflection Count Labels			
			CreateWindow("STATIC", "Reflection Count", WS_VISIBLE | WS_CHILD | SS_LEFT, 115, 10, 120, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 36, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "3", WS_VISIBLE | WS_CHILD | SS_LEFT, 165, 36, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "6", WS_VISIBLE | WS_CHILD | SS_LEFT, 255, 36, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);			
			CreateWindow("STATIC", "Count", WS_VISIBLE | WS_CHILD | SS_LEFT, 33, 50, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);			

			// Refraction Count Labels			
			CreateWindow("STATIC", "Refraction Count", WS_VISIBLE | WS_CHILD | SS_LEFT, 115, 100, 120, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 126, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "3", WS_VISIBLE | WS_CHILD | SS_LEFT, 165, 126, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "6", WS_VISIBLE | WS_CHILD | SS_LEFT, 255, 126, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "Count", WS_VISIBLE | WS_CHILD | SS_LEFT, 33, 140, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

			// Fresnel Power Labels			
			CreateWindow("STATIC", "Fresnel Power", WS_VISIBLE | WS_CHILD | SS_LEFT, 115, 200, 120, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "1.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 80, 226, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "3.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 160, 226, 20, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "5.0", WS_VISIBLE | WS_CHILD | SS_LEFT, 248, 226, 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
			CreateWindow("STATIC", "Count", WS_VISIBLE | WS_CHILD | SS_LEFT, 33, 240, 80, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

			// Confirmation button
			CreateWindow("BUTTON", "OK", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 116, 290, 100, 30, hwnd, (HMENU)BUTTON_ID, GetModuleHandle(NULL), NULL);

			// Reflection Count Trackbars
			reflectionCountTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Reflection", 70, 50, Scene::get().getReflectionCount(), 6);
			refractionCountTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Refraction", 70, 140, Scene::get().getRefractionCount(), 6);
			fresnelPowerTrackbar = CreateTrackbar(hwnd, GetModuleHandle(NULL), "Fresnel", 70, 240, static_cast<uint32>((Scene::get().getFresnelPower() - 1) * 25), 100);

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
			const auto hi = HIWORD(wParam);

			if (lo == SB_THUMBTRACK || lo == SB_THUMBPOSITION)
			{
				if (lParam == (LPARAM)reflectionCountTrackbar)
				{
					Scene::get().setReflectionCount(hi);
				}
				else if (lParam == (LPARAM)refractionCountTrackbar)
				{
					Scene::get().setRefractionCount(hi);
				}
				else if (lParam == (LPARAM)fresnelPowerTrackbar)
				{
					Scene::get().setFresnelPower(hi/25.0f + 1);
				}

				PostMessage(GetParent(hwnd), WM_HSCROLL, wParam, lParam);
			}
		} break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
		{
			CreateMenus(hwnd);
		} break;

		case WM_DESTROY:
		{
			PostQuitMessage(0);
		} break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND WINAPI win32::CreatePlanesEditDialog(HWND hwnd, HINSTANCE hInstance, const uint32 planeIndex)
{
	currentPlaneIndex = planeIndex;

	WNDCLASS wc = {};
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = PlanesEditWndProc;
	wc.lpszClassName = "PlanesEdit";
	wc.lpszMenuName = 0;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&wc);


	const auto width = 330;
	const auto height = 690;
	const auto x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	const auto y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

	auto hwndPlanesEdit = CreateWindow(
		wc.lpszClassName,
		"Edit Planes",
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
		x, y,
		width, height,
		hwnd,
		NULL,
		hInstance,
		NULL
	);

	SetFocus(hwndPlanesEdit);
	ShowWindow(hwndPlanesEdit, SW_SHOW);
	return hwndPlanesEdit;
}

HWND WINAPI win32::CreateSpheresEditDialog(HWND hwnd, HINSTANCE hInstance, const uint32 sphereIndex)
{
	currentSphereIndex = sphereIndex;

	WNDCLASS wc = {};
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = SpheresEditWndProc;
	wc.lpszClassName = "SpheresEdit";
	wc.lpszMenuName = 0;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&wc);


	const auto width = 330;
	const auto height = 690;
	const auto x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	const auto y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

	auto hwndSpheresEdit = CreateWindow(
		wc.lpszClassName,
		"Edit Spheres",
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
		x, y,
		width, height,
		hwnd,
		NULL,
		hInstance,
		NULL
	);	

	SetFocus(hwndSpheresEdit);
	ShowWindow(hwndSpheresEdit, SW_SHOW);
	return hwndSpheresEdit;
}

HWND WINAPI win32::CreateLightsEditDialog(HWND hwnd, HINSTANCE hInstance, const uint32 lightIndex)
{
	currentLightIndex = lightIndex;

	WNDCLASS wc = {};
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = LightEditWndProc;
	wc.lpszClassName = "LightEdit";
	wc.lpszMenuName = 0;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&wc);


	const auto width = 330;
	const auto height = Scene::get().getLight(lightIndex).getLightType() == Light::DIR_LIGHT ? 490 : 580;
	const auto x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	const auto y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

	auto hwndLightEdit = CreateWindow(
		wc.lpszClassName,
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

HWND WINAPI win32::CreateReflectionAndRefractionCountDialog(HWND hwnd, HINSTANCE hInstance)
{
	WNDCLASS wc = {};
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = ReflectionCountWndProc;
	wc.lpszClassName = "ReflectionRefractionCount";
	wc.lpszMenuName = 0;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&wc);


	const auto width = 330;
	const auto height = 370;
	const auto x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	const auto y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

	auto hwndReflectionRefractionCount = CreateWindow(
		wc.lpszClassName,
		"Change Reflection & Refraction Count",
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
		x, y,
		width, height,
		hwnd,
		NULL,
		hInstance,
		NULL
	);

	SetFocus(hwndReflectionRefractionCount);
	ShowWindow(hwndReflectionRefractionCount, SW_SHOW);
	return hwndReflectionRefractionCount;
}

HWND WINAPI win32::CreateMainWindow(HINSTANCE instance, const sint32 windowWidth, const sint32 windowHeight, const std::string& title)
{
	// Window Registration
	WNDCLASS wc = {};
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hInstance = instance;
	wc.lpfnWndProc = MainWindowProc;
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

void WINAPI win32::CreateIODialog(HWND hwnd, HINSTANCE instance, const IO_DIALOG_TYPE ioDialogType, io_result_callback callbackOnIOCompletion)
{
	OPENFILENAME ofn = {};

	CHAR szCurrentPath[MAX_PATH + 1];
	CHAR szFileFullPath[MAX_PATH + 1] = "";
	CHAR szFileTitle[MAX_PATH + 1] = "scene.scn";
	CHAR szCustomFilter[MAX_PATH + 1] = "Scene Files (*.scn)\0*.scn\0";
	GetModuleFileName(NULL, szCurrentPath, MAX_PATH + 1);

	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.hInstance = instance;
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFileFullPath;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.lpstrFilter = szCustomFilter;
	ofn.lpstrInitialDir = szCurrentPath;
	ofn.lpstrTitle = ioDialogType == SAVE_AS ? "Save Scene As" : "Open Scene";
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrDefExt = "scn";
	ofn.nMaxCustFilter = MAX_PATH;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;

	if (ioDialogType == SAVE_AS && GetSaveFileName(&ofn))
	{		
		Scene::get().saveScene(szFileFullPath, callbackOnIOCompletion);
	}
	else if (ioDialogType == OPEN && GetOpenFileName(&ofn))
	{
		Scene::get().openScene(szFileFullPath, callbackOnIOCompletion);
	}
}
