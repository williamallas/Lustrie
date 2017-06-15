#pragma once

#include <Windows.h>

#include "LustrieCore.h"
#include "core/NonCopyable.h"

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
	LPCSTR _applicationName;
	HINSTANCE _hinstance;
	HWND _hwnd;
	bool _fullScreen;
	bool _escapePressed = false;
	bool _isInit = false;
	EventManager _eventManager;

	LustrieCore _game;
};

static System* ApplicationHandle = 0;
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

