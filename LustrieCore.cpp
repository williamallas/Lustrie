#include "LustrieCore.h"

#include <core/Chrono.h>
#include <core/ctpl_stl.h>

ctpl::thread_pool g_threadPool(8);

LustrieCore::LustrieCore(EventManager& event) : _event(event)
{
	_camera.position = vec3(0, 0, 150);
}

bool LustrieCore::init(tim::ivec2 resolution, bool fullscreen, HWND handle)
{
	srand(time(NULL));
	_texGen = eastl::make_unique<TextureGenerator>(rand());

	bool b = _graphics.init(resolution, fullscreen, handle);

	eastl::string shaderSrc = eastl::string(dx12::g_headerShader) + eastl::string(dx12::g_planetShader);
	_planet.planetMaterial = eastl::make_unique<Material>(_graphics.createTexturedForwardMaterial(shaderSrc.c_str(), 
		eastl::make_shared<TexturePool>(Graphics::SIZE_TEXTURE_POOL), true, false));

	auto grassMat = eastl::make_shared<Material>(_graphics.createPointToTriangleGSForwardMaterial(dx12::g_grassShader, _planet.planetMaterial->texturePool(), false, false));
	_planet.grassMaterial.push_back(grassMat);

	shaderSrc = eastl::string(dx12::g_headerShader) + eastl::string(dx12::g_defaultShader);
	_planet.plantMaterial = eastl::make_unique<Material>(_graphics.createTexturedForwardMaterial(shaderSrc.c_str(), _planet.planetMaterial->texturePool(), true, false));
	_planet.leafMaterial = eastl::make_unique<Material>(_graphics.createTexturedForwardMaterial(shaderSrc.c_str(), _planet.planetMaterial->texturePool(), false, false));
	
	auto sync = g_threadPool.push([&](int) {
		auto gen = eastl::make_unique<tim::FractalNoise<tim::WorleyNoise<tim::vec3>>>(3, WorleyNoiseInstancer<WorleyNoise<vec3>>(50, 8, 1, rand()));
		eastl::swap(gen, g_fractalWorley3d);
	});

	sync.wait();

#ifndef _DEBUG
	TextureGenerator::genWorleyNoise();
#endif

	return b;
}

void LustrieCore::close()
{
	_graphics.close();
}

namespace
{
	bvec4 toColb(const vec4& col) { return { (byte)(col[0] * 255.f + 0.5f), (byte)(col[1] * 255.f + 0.5f), (byte)(col[2] * 255.f + 0.5f), (byte)(col[3] * 255.f + 0.5f) }; }
};


void LustrieCore::update()
{
	Chrono timer;
	//Sleep(10);

	if (_planet.planet == nullptr)
	{
		Planet::Parameter planetParam = Planet::Parameter::generate(rand());
		/*planetParam.sizePlanet = { 60,30 };
		planetParam.largeRidgeZScale = 0;
		planetParam.largeWorleyZScale = 2;
		planetParam.largeWorleyCoef = 1;
		planetParam.simplexDetailZScale = 0.015;
		planetParam.floorHeight = 0.7f;*/

		_camera.position = vec3(0, 0, planetParam.sizePlanet.x()+planetParam.sizePlanet.y());
		_planet.planet = eastl::unique_ptr<Planet>(new Planet(256, planetParam, rand()));

#ifdef _DEBUG
		for (int i = 0; i<1; ++i)
#else
		for(int i=0 ; i<4 ; ++i)
#endif
			_planet.grassOnPlanet.emplace_back(eastl::unique_ptr<PlanetGrass>(new PlanetGrass(*_planet.planet, i)));
		
		_planet.plants = eastl::make_unique<PlanetPlants>(rand());
		_planet.plants->createTree(1, 5);
		_planet.plants->createTree(1, 5);
		for (int i = 0; i<10; ++i)
			_planet.plants->populatePlant(*_planet.planet, i, 30);
	}

	if (_event.key('Q').firstPress)
	{
	#define RESO 1024
		int seed = rand();
		g_threadPool.push([&, seed](int) {
			std::mt19937 randEngine(seed);

			eastl::vector<TextureGenerator::ColorBank> colorTypes;
			colorTypes.push_back((TextureGenerator::ColorBank)(randEngine() % TextureGenerator::ColorBank::NB_COLOR));
			if(randEngine()%2)
				colorTypes.push_back((TextureGenerator::ColorBank)(randEngine() % TextureGenerator::ColorBank::NB_COLOR));

			Palette mainPalette = _texGen->genPalette(colorTypes, { 3,6 });
			auto img = _texGen->genGroundTexture(RESO, mainPalette);
			_planet.planetMaterial->texturePool()->setTexture(PlanetSystem::TEX_PLANET, Graphics::createTextureWithMips(img));

			auto img2 = _texGen->genGrassTexture(RESO/2, mainPalette);
			_planet.grassMaterial[0]->texturePool()->setTexture(PlanetSystem::TEX_PLANET_GRASS, Graphics::createTextureWithMips(img2));

			auto colorTypesForTrunk = colorTypes;
			colorTypesForTrunk[0] = TextureGenerator::ColorBank::BROWN;
			_planet.plantMaterial->texturePool()->setTexture(PlanetSystem::TEX_PLANET_PLANT_TRUNK, 
				Graphics::createTextureWithMips(_texGen->genTreeBarkTexture(RESO, _texGen->genPalette(colorTypesForTrunk, { 3,4 }))));
			_planet.plantMaterial->texturePool()->setTexture(PlanetSystem::TEX_PLANET_PLANT_LEAF, 
				Graphics::createTextureWithMips(_texGen->genLeafTexture(RESO/2, _texGen->genPalette(colorTypes, { 3,4 }) )));

			img2.exportBMP("grass.bmp", [](auto x) { return x.to<3>(); });
			img.exportBMP("ground.bmp", [](auto x) { return x.to<3>(); });
			_texGen->genLeafTexture(1024, _texGen->randPalette()).exportBMP("leaf.bmp", [](auto x) { return x.to<3>(); });
		});
	}

	_graphics.frame(_camera, _planet);

	_camera.up = _planet.planet->computeUp(_camera.position);
	_camera.update(_event, _elapsed);

	_elapsed = (float)timer.elapsed().to_secs();
	_totalTime += _elapsed;
	_graphics.setTimeElapsed(_totalTime);

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