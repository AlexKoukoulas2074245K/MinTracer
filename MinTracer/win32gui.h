/********************************************************************/
/** win32gui.h by Alex Koukoulas (C) 2017 All Rights Reserved  **/
/** File Description:                                              **/
/********************************************************************/

#pragma once

// Local Headers
#include "typedefs.h"

// Remote Headers
#include <Windows.h>
#include <Commctrl.h>

#include <string>

const uint32 GUID_QUIT_SCENE = 11;
const uint32 GUID_REFL_REFR_COUNT_RENDER = 31; 
const uint32 GUID_RESTART_RENDER = 32;
const uint32 LIGHT_GUID_OFFSET = 100;
const uint32 SPHERE_GUID_OFFSET = 200;
const uint32 PLANE_GUID_OFFSET = 300;

void CreateMenus(HWND hwnd);

HWND WINAPI CreatePlanesEditDialog(HWND hwnd, HINSTANCE hInstance, const uint32 planeIndex);

HWND WINAPI CreateSpheresEditDialog(HWND hwnd, HINSTANCE hInstance, const uint32 sphereIndex);

HWND WINAPI CreateLightsEditDialog(HWND hwnd, HINSTANCE hInstance, const uint32 lightIndex);

HWND WINAPI CreateReflectionAndRefractionCountDialog(HWND hwnd, HINSTANCE hInstance);

HWND WINAPI CreateMainWindow(HINSTANCE instance, const sint32 windowWidth, const sint32 windowHeight, const std::string& title);
