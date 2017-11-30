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
const uint32 GUID_LIGHTS_EDIT = 21;
const uint32 GUID_OBJECTS_EDIT = 22;
const uint32 GUID_RESTART_RENDER = 31;

void CreateMenus(HWND hwnd);

HWND WINAPI CreateObjectsEditDialog(HWND hwnd, HINSTANCE hInstance);

HWND WINAPI CreateLightEditDialog(HWND hwnd, HINSTANCE hInstance);

HWND WINAPI CreateMainWindow(HINSTANCE instance, const sint32 windowWidth, const sint32 windowHeight, const std::string& title);
