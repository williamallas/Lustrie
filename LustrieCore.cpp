#include "LustrieCore.h"
#include "TextureGenerator.h"

#include <core/Chrono.h>
#include <core/ctpl_stl.h>

ctpl::thread_pool g_threadPool(8);

LustrieCore::LustrieCore(EventManager& event) : _event(event)
{
	_camera.position = vec3(0, 0, 150);
}

bool LustrieCore::init(tim::ivec2 resolution, bool fullscreen, HWND handle)
{
	bool b = _graphics.init(resolution, fullscreen, handle);

	eastl::string shaderSrc = eastl::string(dx12::g_headerShader) + eastl::string(dx12::g_planetShader);
	_material = eastl::make_unique<Material>(_graphics.createTexturedForwardMaterial(shaderSrc.c_str(), 16, true, false));

	return b;
}

void LustrieCore::close()
{
	_graphics.close();
}

void LustrieCore::update()
{
	Chrono timer;
	Sleep(10);

	if (_planet == nullptr)
	{
		Planet::Parameter planetParam;
		planetParam.sizePlanet = { 60,25 };
		planetParam.largeSimplexCoef = 2;
		_camera.position = vec3(0, 0, planetParam.sizePlanet.x()+planetParam.sizePlanet.y());
		_planet = eastl::unique_ptr<Planet>(new Planet(256, planetParam));

		/*
		dx12::Texture* t = new dx12::Texture({ 512,512 });
		auto img = TextureGenerator(34534).genGroundTexture(512);
		t->upload(reinterpret_cast<byte*>(img.data()));
		_graphics.renderer().setTextureRes(t);*/
	}

	if (_event.key('Q').firstPress)
	{
		int seed = rand();
		g_threadPool.push([&, seed](int) {
			dx12::Texture* t = new dx12::Texture({ 1024,1024 });
			auto img = TextureGenerator(seed+13).genGroundTexture(1024);
			img.exportBMP("ground.bmp", [](auto x) { return x.to<3>(); });
			t->upload(reinterpret_cast<byte*>(img.data()));
			_material->texturePool()->setTexture(0, ProxyTexture(t));
		});
	}

	_graphics.frame(_camera, *_material, *_planet);

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