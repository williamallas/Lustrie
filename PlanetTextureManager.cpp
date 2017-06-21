#include "PlanetTextureManager.h"

using namespace tim;

PlanetTextureManager::PlanetTextureManager(int seed) : _seed(seed)
{
	_textures = eastl::make_unique<TexturePool>(16);
}