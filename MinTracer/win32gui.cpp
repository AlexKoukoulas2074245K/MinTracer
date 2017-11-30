/**********************************************************************/
/** win32gui.cpp by Alex Koukoulas (C) 2017 All Rights Reserved      **/
/** File Description:                                                **/
/**********************************************************************/

#pragma once

// Local Headers
#include "win32gui.h"

// Remote Headers

// Use Modern GUI style
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


HWND lightPositionX, lightPositionY, lightPositionZ;
HWND lightColorX, lightColorY, lightColorZ;
HWND sphereOffsetX, sphereOffsetY, sphereOffsetZ;
HWND sphereRadius;
HWND sphereGlossiness;

static HWND WINAPI CreateTrackbar(
	HWND hwndDlg,        // handle of dialog box (parent window) 
	HINSTANCE hInstance, // Instance	
	const std::string& title,
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

void CreateMenus(HWND hwnd)
{
	HMENU hMenubar = CreateMenu();
	HMENU hSceneMenu = CreateMenu();
	HMENU hEditMenu = CreateMenu();
	HMENU hRenderMenu = CreateMenu();

	// Scene Menu	
	AppendMenuW(hSceneMenu, MF_STRING, GUID_QUIT_SCENE, L"&Quit");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hSceneMenu, L"&Scene");

	// Edit Menu
	AppendMenuW(hEditMenu, MF_STRING, GUID_LIGHTS_EDIT, L"&Light");
	AppendMenuW(hEditMenu, MF_STRING, GUID_OBJECTS_EDIT, L"&Objects");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hEditMenu, L"&Edit");

	// Render Menu
	AppendMenuW(hRenderMenu, MF_STRING, GUID_RESTART_RENDER, L"&Restart Rendering");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hRenderMenu, L"&Render");

	// Master Menu Bar
	SetMenu(hwnd, hMenubar);
}

LRESULT CALLBACK LightEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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


LRESULT CALLBACK ObjectsEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

HWND WINAPI CreateLightEditDialog(HWND hwnd, HINSTANCE hInstance)
{

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

HWND WINAPI CreateObjectsEditDialog(HWND hwnd, HINSTANCE hInstance)
{

	WNDCLASS wc = {};
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = ObjectsEditWndProc;
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

HWND WINAPI CreateMainWindow(HINSTANCE instance, const sint32 windowWidth, const sint32 windowHeight, const std::string& title)
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