#include "System.h"

using namespace tim;

System::System()
{
	init();
}


System::~System()
{
	close();
}


void System::run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	bool done = false;
	while (!done && _isInit)
	{
		// handle the windows messages
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (_escapePressed)
			done = true;

		if (msg.message == WM_QUIT)
			done = true;

		_graphics.frame();
	}

	return;
}


void System::close()
{
	if (_isInit)
	{
		_isInit = false;
		_graphics.close();
		closeWindow();
	}
}


bool System::init()
{
	const bool FULLSCREEN = false;
	const ivec2 RESOLUTION = { 1280,720 };
	openWindow(RESOLUTION, FULLSCREEN);

	_isInit = _graphics.init(RESOLUTION, FULLSCREEN, _hwnd);

	return _isInit;
}


void System::openWindow(ivec2 resolution, bool fullScreen)
{
	ApplicationHandle = this;

	_hinstance = GetModuleHandle(NULL);
	_applicationName = L"Dx12Test";
	_fullScreen = fullScreen;
	_escapePressed = false;

	// setup the windows class with default settings
	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = _hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = _applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&wc);

	if (fullScreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsHeight = (unsigned long)resolution.y();
		dmScreenSettings.dmPelsWidth = (unsigned long)resolution.x();
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// change the display settings to full screen
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
	}

	// create the window with the screen settings and get the handle to it
	_hwnd = CreateWindowEx(WS_EX_APPWINDOW, _applicationName, _applicationName,
						   WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP, 100, 100, resolution.x(), resolution.y(), NULL, NULL, _hinstance, NULL);

	// bring the window up on the screen and set it as main focus
	ShowWindow(_hwnd, SW_SHOW);
	SetForegroundWindow(_hwnd);
	SetFocus(_hwnd);
}


void System::closeWindow()
{
	if (_hwnd == NULL)
		return;

	// fix the display settings if leaving full screen mode
	if (_fullScreen)
		ChangeDisplaySettings(NULL, 0);

	// remove the window
	DestroyWindow(_hwnd);
	_hwnd = NULL;

	// remove the application instance
	UnregisterClass(_applicationName, _hinstance);
	_hinstance = NULL;

	// Release the pointer to this class.
	ApplicationHandle = NULL;
}


LRESULT CALLBACK System::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
	case WM_KEYDOWN:
		if (wparam == VK_ESCAPE)
			_escapePressed = true;
		return 0;

	default: 
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
	// Check if the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	// Check if the window is being closed.
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	// All other messages pass to the message handler in the system class.
	default:
		return ApplicationHandle->MessageHandler(hwnd, umsg, wparam, lparam);
	}
}

