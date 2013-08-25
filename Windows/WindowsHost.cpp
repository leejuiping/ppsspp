// Copyright (c) 2012- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#include <algorithm>

#include "base/NativeApp.h"
#include "input/input_state.h"
#include "input/keycodes.h"

#include "Core/Core.h"
#include "Core/Config.h"
#include "Core/CoreParameter.h"
#include "Core/System.h"
#include "EmuThread.h"
#include "DSoundStream.h"
#include "WindowsHost.h"
#include "WndMainWindow.h"
#include "OpenGLBase.h"

#include "Windows/Debugger/DebuggerShared.h"
#include "Windows/Debugger/Debugger_Disasm.h"
#include "Windows/Debugger/Debugger_MemoryDlg.h"
#include "Windows/InputBox.h"

#include "Windows/DinputDevice.h"
#include "Windows/XinputDevice.h"
#include "Windows/KeyboardDevice.h"

#include "Core/Debugger/SymbolMap.h"

#include "Common/StringUtils.h"
#include "file/file_util.h"
#include "main.h"

static PMixer *curMixer;

float mouseDeltaX = 0;
float mouseDeltaY = 0;

int MyMix(short *buffer, int numSamples, int bits, int rate, int channels)
{
	if (curMixer && !Core_IsStepping())
		return curMixer->Mix(buffer, numSamples);
	else
	{
		memset(buffer,0,numSamples*sizeof(short)*2);
		return numSamples;
	}
}

static BOOL PostDialogMessage(Dialog *dialog, UINT message, WPARAM wParam = 0, LPARAM lParam = 0)
{
	return PostMessage(dialog->GetDlgHandle(), message, wParam, lParam);
}

WindowsHost::WindowsHost(HWND mainWindow, HWND displayWindow)
{
	mainWindow_ = mainWindow;
	displayWindow_ = displayWindow;
	mouseDeltaX = 0;
	mouseDeltaY = 0;

#define PUSH_BACK(Cls) do { list.push_back(std::shared_ptr<InputDevice>(new Cls())); } while (0)

	input.push_back(std::shared_ptr<InputDevice>(new XinputDevice()));
	input.push_back(std::shared_ptr<InputDevice>(new DinputDevice()));
	keyboard = std::shared_ptr<KeyboardDevice>(new KeyboardDevice());
	input.push_back(keyboard);

	SetConsolePosition();
}

bool WindowsHost::InitGL(std::string *error_message)
{
	return GL_Init(MainWindow::GetDisplayHWND(), error_message);
}

void WindowsHost::ShutdownGL()
{
	GL_Shutdown();
}

void WindowsHost::SetWindowTitle(const char *message)
{
	std::string title = std::string("PPSSPP ") + PPSSPP_GIT_VERSION;
	if (message)
		title = title + " - " + message;

	int size = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), (int) title.size(), NULL, 0);
	if (size > 0)
	{
		// VC++6.0 any more?
		wchar_t *utf16_title = new(std::nothrow) wchar_t[size + 1];
		if (utf16_title)
			size = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), (int) title.size(), utf16_title, size);
		else
			size = 0;

		if (size > 0)
		{
			utf16_title[size] = 0;
			// Don't use SetWindowTextW because it will internally use DefWindowProcA.
			DefWindowProcW(mainWindow_, WM_SETTEXT, 0, (LPARAM) utf16_title);
			delete[] utf16_title;
		}
	}

	// Something went wrong, fall back to using the local codepage.
	if (size <= 0)
		SetWindowTextA(mainWindow_, title.c_str());
}

void WindowsHost::InitSound(PMixer *mixer)
{
	curMixer = mixer;
	DSound::DSound_StartSound(MainWindow::GetHWND(), MyMix);
}

void WindowsHost::UpdateSound()
{
	DSound::DSound_UpdateSound();
}

void WindowsHost::ShutdownSound()
{
	DSound::DSound_StopSound();
	if (curMixer != NULL)
		delete curMixer;
	curMixer = 0;
}

void WindowsHost::UpdateUI()
{
	MainWindow::Update();
}


void WindowsHost::UpdateMemView() 
{
	for (int i=0; i<numCPUs; i++)
		if (memoryWindow[i])
			PostDialogMessage(memoryWindow[i], WM_DEB_UPDATE);
}

void WindowsHost::UpdateDisassembly()
{
	for (int i=0; i<numCPUs; i++)
		if (disasmWindow[i])
			PostDialogMessage(disasmWindow[i], WM_DEB_UPDATE);
}

void WindowsHost::SetDebugMode(bool mode)
{
	for (int i = 0; i < numCPUs; i++)
	{
		if (disasmWindow[i])
			PostDialogMessage(disasmWindow[i], WM_DEB_SETDEBUGLPARAM, 0, (LPARAM)mode);
	}
}

extern BOOL g_bFullScreen;

void WindowsHost::PollControllers(InputState &input_state)
{
	bool doPad = true;
	for (auto iter = this->input.begin(); iter != this->input.end(); iter++)
	{
		auto device = *iter;
		if (!doPad && device->IsPad())
			continue;
		if (device->UpdateState(input_state) == InputDevice::UPDATESTATE_SKIP_PAD)
			doPad = false;
	}

	mouseDeltaX *= 0.9f;
	mouseDeltaY *= 0.9f;

	// TODO: Tweak!
	float mx = std::max(-1.0f, std::min(1.0f, mouseDeltaX * 0.01f));
	float my = std::max(-1.0f, std::min(1.0f, mouseDeltaY * 0.01f));
	AxisInput axisX, axisY;
	axisX.axisId = JOYSTICK_AXIS_MOUSE_REL_X;
	axisX.deviceId = DEVICE_ID_MOUSE;
	axisX.value = mx;
	axisY.axisId = JOYSTICK_AXIS_MOUSE_REL_Y;
	axisY.deviceId = DEVICE_ID_MOUSE;
	axisY.value = my;

	// Disabled for now as it makes the mapping dialog unusable!
	//if (fabsf(mx) > 0.1f) NativeAxis(axisX);
	//if (fabsf(my) > 0.1f) NativeAxis(axisY);
}

void WindowsHost::BootDone()
{
	symbolMap.SortSymbols();
	SendMessage(MainWindow::GetHWND(), WM_USER+1, 0,0);

	SetDebugMode(!g_Config.bAutoRun);
	Core_EnableStepping(!g_Config.bAutoRun);
}

static std::string SymbolMapFilename(const char *currentFilename, char* ext)
{
	FileInfo info;

	std::string result = currentFilename;

	// can't fail, definitely exists if it gets this far
	getFileInfo(currentFilename, &info);
	if (info.isDirectory)
	{
#ifdef _WIN32
		char* slash = "\\";
#else
		char* slash = "/";
#endif
		if (!endsWith(result,slash))
			result += slash;

		return result + ".ppsspp-symbols" + ext;
	} else {
		size_t dot = result.rfind('.');
		if (dot == result.npos)
			return result + ext;

		result.replace(dot, result.npos, ext);
		return result;
	}
}

bool WindowsHost::AttemptLoadSymbolMap()
{
	bool result1 = symbolMap.LoadSymbolMap(SymbolMapFilename(PSP_CoreParameter().fileToStart.c_str(),".map").c_str());
	bool result2 = symbolMap.LoadNocashSym(SymbolMapFilename(PSP_CoreParameter().fileToStart.c_str(),".sym").c_str());
	return result1 || result2;
}

void WindowsHost::SaveSymbolMap()
{
	symbolMap.SaveSymbolMap(SymbolMapFilename(PSP_CoreParameter().fileToStart.c_str(),".map").c_str());
}

void WindowsHost::AddSymbol(std::string name, u32 addr, u32 size, int type=0) 
{
	symbolMap.AddSymbol(name.c_str(), addr, size, (SymbolType)type);
}

bool WindowsHost::IsDebuggingEnabled()
{
#ifdef _DEBUG
	return true;
#else
	return false;
#endif
}

void WindowsHost::SetConsolePosition()
{
	HWND console = GetConsoleWindow();
	if (console != NULL && g_Config.iConsoleWindowX != -1 && g_Config.iConsoleWindowY != -1)
		SetWindowPos(console, NULL, g_Config.iConsoleWindowX, g_Config.iConsoleWindowY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void WindowsHost::UpdateConsolePosition()
{
	RECT rc;
	HWND console = GetConsoleWindow();
	if (console != NULL && GetWindowRect(console, &rc))
	{
		g_Config.iConsoleWindowX = rc.left;
		g_Config.iConsoleWindowY = rc.top;
	}
}

bool WindowsHost::InputBoxGetString(char *title, const char *defaultValue, char *outValue, size_t outLength)
{
	return InputBox_GetString(MainWindow::GetHInstance(), MainWindow::GetHWND(), title, defaultValue, outValue, outLength);
}
