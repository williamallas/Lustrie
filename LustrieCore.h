#pragma once

#include "math/Vector.h"
#include "EventManager.h"
#include "graphics\Graphics.h"
#include "ControlCamera.h"
#include "PlanetSystem.h"
#include "geometry\Palette.h"
#include "TextureGenerator.h"

#include <core/ctpl_stl.h>
extern ctpl::thread_pool g_threadPool;

class LustrieCore
{
public:
	LustrieCore(EventManager& event);

	bool init(tim::ivec2, bool, HWND);
	void close();
	void update();

private:
	EventManager& _event;
	Graphics _graphics;
	ControlCamera _camera;

	eastl::unique_ptr<TextureGenerator> _texGen;
	PlanetSystem _planet;

	float _elapsed = 0.01f;
	float _totalTime = 0;
};