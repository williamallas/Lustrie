#include "TextureGenerator.h"
#include "driver/DX12Texture.h"

using namespace tim;

FractalNoise<WorleyNoise<vec2>> g_fractalWorley[4] = { FractalNoise<WorleyNoise<vec2>>(4, WorleyNoiseInstancer<WorleyNoise<vec2>>(50, 4)),
													   FractalNoise<WorleyNoise<vec2>>(4, WorleyNoiseInstancer<WorleyNoise<vec2>>(100, 4)),
													   FractalNoise<WorleyNoise<vec2>>(4, WorleyNoiseInstancer<WorleyNoise<vec2>>(200, 4)),
													   FractalNoise<WorleyNoise<vec2>>(4, WorleyNoiseInstancer<WorleyNoise<vec2>>(400, 4)) };

namespace
{
	bvec4 toColb(const vec4& col) { return { (byte)(col[0]*255.f + 0.5f), (byte)(col[1] *256.f + 0.5f), (byte)(col[2] *255.f + 0.5f) }; }
};

ImageAlgorithm<bvec4> TextureGenerator::genGroundTexture(uint res)
{
	Palette palette = randPalette();

	auto bmpconvert = [](float x) { return bvec3(x * 255, x * 255, x * 255); };
	auto img = g_fractalWorley[3]. //FractalNoise<SimplexNoise2D>(6, SimplexNoiseInstancer<SimplexNoise2D>(float(8 << (_randEngine() % 2)), 2, _seed + 3249875)).
		generate({ res,res }).map([](float x) { return 1.f - x; });

	return img.makeTilable().map(palette);
}

Palette TextureGenerator::randPalette(uivec2 nbColorRange, uint satFrom)
{
	std::uniform_int_distribution<uint> randInt(nbColorRange.x(), nbColorRange.y());
	Palette palette;
	uint nbColor = randInt(_randEngine);

	if (nbColor == 0)
		nbColor++;

	for (uint i = 0; i < nbColor; ++i)
	{
		vec4 col = randColorf();
		if (i >= satFrom) col.saturate();

		palette.setColor(toColb(col), float(i) / (nbColor - 1));
	}

	return palette;
}

tim::bvec4 TextureGenerator::randColor()
{
	return { (byte)(_randEngine() % 256), (byte)(_randEngine() % 256) , (byte)(_randEngine() % 256), 255 };
}

tim::vec4 TextureGenerator::randColorf()
{
	return { _random(_randEngine), _random(_randEngine) , _random(_randEngine), 1 };
}