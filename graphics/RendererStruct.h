#pragma once

#include "math/Vector.h"
#include <Windows.h>

struct InitRendererInfo
{
	tim::ivec2 resolution;
	bool fullscreen;
	bool vsync;
	HWND windowHandle;
};

enum class ShaderType { VERTEX, PIXEL, GEOMETRY, COMPUTE };