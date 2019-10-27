#pragma once

#include "math/Vector.h"
#include "math/Matrix.h"
#include <Windows.h>

struct InitRendererInfo
{
	tim::ivec2 resolution;
	bool fullscreen;
	bool vsync;
	HWND windowHandle;
};

enum class ShaderType { VERTEX, PIXEL, GEOMETRY, COMPUTE };

struct MaterialParameter
{
	tim::ivec4 textures;
	tim::vec4 parameter;
};

class MeshBuffers;

struct ObjectInstance
{
	MeshBuffers* mesh;
	tim::mat4 tranform;
	MaterialParameter parameter;
};