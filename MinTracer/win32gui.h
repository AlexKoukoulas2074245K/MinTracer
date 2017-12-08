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
#include <functional>

namespace win32
{
	enum IO_DIALOG_TYPE
	{
		OPEN, SAVE_AS
	};

	enum IO_DIALOG_RESULT_TYPE
	{
		SUCCESS, FAILURE
	};

	using io_result_callback = std::function<void(const IO_DIALOG_RESULT_TYPE)>;

	const uint32 GUID_OPEN_SCENE = 11;
	const uint32 GUID_SAVE_SCENE = 12;
	const uint32 GUID_QUIT_SCENE = 13;
	const uint32 GUID_REFL_REFR_COUNT_RENDER = 31;
	const uint32 GUID_RESTART_RENDER = 32;
	const uint32 LIGHT_GUID_OFFSET = 100;
	const uint32 SPHERE_GUID_OFFSET = 200;
	const uint32 PLANE_GUID_OFFSET = 300;
	
	HWND WINAPI CreatePlanesEditDialog(HWND hwnd, HINSTANCE hInstance, const uint32 planeIndex);

	HWND WINAPI CreateSpheresEditDialog(HWND hwnd, HINSTANCE hInstance, const uint32 sphereIndex);

	HWND WINAPI CreateLightsEditDialog(HWND hwnd, HINSTANCE hInstance, const uint32 lightIndex);

	HWND WINAPI CreateReflectionAndRefractionCountDialog(HWND hwnd, HINSTANCE hInstance);

	HWND WINAPI CreateMainWindow(HINSTANCE instance, const sint32 windowWidth, const sint32 windowHeight, const std::string& title);

	void WINAPI CreateIODialog(HWND hwnd, HINSTANCE instance, const IO_DIALOG_TYPE ioDialogType, io_result_callback callbackOnIOCompletion);
}