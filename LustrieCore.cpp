#include "LustrieCore.h"
#include "TextureGenerator.h"

#include <core/Chrono.h>
#include <core/ctpl_stl.h>

ctpl::thread_pool g_threadPool(6);

LustrieCore::LustrieCore(EventManager& event) : _event(event)
{
	_camera.position = vec3(0, 0, 150);
}

bool LustrieCore::init(tim::ivec2 resolution, bool fullscreen, HWND handle)
{
	TextureGenerator(23452345).genGroundTexture(512);
	return _graphics.init(resolution, fullscreen, handle);
}

void LustrieCore::close()
{
	_graphics.close();
}

void LustrieCore::update()
{
	Chrono timer;

	if (_planet == nullptr)
	{
		Planet::Parameter planetParam;
		planetParam.sizePlanet = { 60,25 };
		planetParam.largeSimplexCoef = 2;
		_camera.position = vec3(0, 0, planetParam.sizePlanet.x()+planetParam.sizePlanet.y());
		_planet = eastl::unique_ptr<Planet>(new Planet(256, planetParam));
	}

	_graphics.frame(_camera, *_planet);

	_camera.up = _planet->computeUp(_camera.position);
	_camera.update(_event, _elapsed);

	_elapsed = (float)timer.elapsed().to_secs();
	_totalTime += _elapsed;

	static int numFrame = 0;
	static float frameTime = 0;

	frameTime += float(timer.elapsed().to_secs());
	numFrame++;
	if (numFrame % 500 == 0)
	{
		std::cout << (numFrame / frameTime) << " fps\n";
		frameTime = 0;
		numFrame = 0;
	}
}