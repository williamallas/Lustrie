#include "TextureGenerator.h"
#include "driver/DX12Texture.h"

using namespace tim;

FractalNoise<WorleyNoise<vec2>> g_fractalWorley = FractalNoise<WorleyNoise<vec2>>(4, WorleyNoiseInstancer<WorleyNoise<vec2>>(100, 4));

namespace
{
	bvec4 toColb(const vec4& col) { return { (byte)(col[0]*255.f + 0.5f), (byte)(col[1] *256.f + 0.5f), (byte)(col[2] *255.f + 0.5f) }; }
};

ImageAlgorithm<bvec4> TextureGenerator::genGroundTexture(uint res)
{
	Palette palette;
	int nbColor = 2 + _randEngine() % 5;

	for (int i = 0; i < nbColor; ++i)
	{
		vec4 col = randColorf();
		if (_randEngine() % 3 > 0) col.saturate();

		palette.setColor(toColb(col), float(i) / (nbColor-1));
	}

	auto bmpconvert = [](float x) { return bvec3(x * 255, x * 255, x * 255); };
	auto img = g_fractalWorley. //FractalNoise<SimplexNoise2D>(6, SimplexNoiseInstancer<SimplexNoise2D>(float(8 << (_randEngine() % 2)), 2, _seed + 3249875)).
		generate({ res,res });

	return img.makeTilable().map(palette);
}

tim::bvec4 TextureGenerator::randColor()
{
	return { (byte)(_randEngine() % 256), (byte)(_randEngine() % 256) , (byte)(_randEngine() % 256), 255 };
}

tim::vec4 TextureGenerator::randColorf()
{
	return { _random(_randEngine), _random(_randEngine) , _random(_randEngine), 1 };
}