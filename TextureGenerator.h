#pragma once

#include "math/Vector.h"
#include "math\FractalNoise.h"
#include "math\WorleyNoise.h"
#include "math\SimplexNoise.h"
#include "geometry\Palette.h"

#include <core/ctpl_stl.h>
extern ctpl::thread_pool g_threadPool;

extern eastl::vector<eastl::unique_ptr<tim::FractalNoise<tim::WorleyNoise<tim::vec2>>>> g_fractalWorley[4];

class TextureGenerator
{
public:
	TextureGenerator(int seed) : _seed(seed), _randEngine(seed), _random(0, 1) {}

	tim::ImageAlgorithm<tim::bvec4> genGroundTexture(tim::uint res, const tim::Palette&);
	tim::ImageAlgorithm<tim::bvec4> genGrassTexture(tim::uint res, const tim::Palette&);
	tim::ImageAlgorithm<tim::bvec4> genTreeBarkTexture(tim::uint res, const tim::Palette&);
	tim::ImageAlgorithm<tim::bvec4> genLeafTexture(tim::uint res, const tim::Palette&);

	tim::Palette randPalette(tim::uivec2 nbColorRange = { 2,5 }, tim::uint saturatedFrom = 2);
	tim::bvec4 randColor();
	tim::vec4 randColorf();
	
	static void genWorleyNoise();

	static eastl::vector<tim::ImageAlgorithm<tim::bvec4>> generateMips(const tim::ImageAlgorithm<tim::bvec4>&, tim::uint nbMips = 0);

	enum ColorBank : int
	{ BROWN = 0, ORANGE, ORANGE2, GREEN, GREEN2, WHITE, PURPLE, PURPLE2, NB_COLOR };

	tim::vec4 getColorFromBank(ColorBank);
	tim::Palette genPalette(const eastl::vector<ColorBank>&, tim::uivec2 nbColors);

private:
	int _seed;
	std::mt19937 _randEngine;
	std::uniform_real_distribution<float> _random;
};