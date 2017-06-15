#pragma once

#include "math/Vector.h"
#include "math\FractalNoise.h"
#include "math\WorleyNoise.h"
#include "math\SimplexNoise.h"
#include "geometry\Palette.h"

#include <core/ctpl_stl.h>
extern ctpl::thread_pool g_threadPool;

class TextureGenerator
{
public:
	TextureGenerator(int seed) : _seed(seed), _randEngine(seed), _random(0, 1) {}

	tim::ImageAlgorithm<tim::bvec4> genGroundTexture(tim::uint res);

	tim::bvec4 randColor();

private:
	int _seed;
	std::mt19937 _randEngine;
	std::uniform_real_distribution<float> _random;
};