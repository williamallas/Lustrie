#pragma once

#include <Windows.h>

#include "Graphics.h"
#include "core/NonCopyable.h"
#include "math/Vector.h"

class System : NonCopyable
{
public:
	System();
	~System();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

	void run();
	void close();

private:
	bool init();

	void openWindow(tim::ivec2, bool);
	void closeWindow();

private:
	LPCWSTR _applicationName;
	HINSTANCE _hinstance;
	HWND _hwnd;
	bool _fullScreen;
	bool _escapePressed = false;
	bool _isInit = false;

	Graphics _graphics;
};

static System* ApplicationHandle = 0;
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

