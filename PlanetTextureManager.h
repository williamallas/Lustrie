#pragma once

#include "TextureGenerator.h"
#include "graphics\TexturePool.h"
#include "geometry\Palette.h"

#include <core/ctpl_stl.h>
extern ctpl::thread_pool g_threadPool;

class PlanetTextureManager
{
public:
	struct Parameter
	{
		tim::Palette palette;
		float varianceColor;
	};

	PlanetTextureManager(int seed);

private:
	int _seed;
	eastl::unique_ptr<TexturePool> _textures;
	eastl::vector<ProxyTexture> _allTextures;
	std::mutex _lock;
};
