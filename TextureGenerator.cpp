#include "TextureGenerator.h"

using namespace tim;


ImageAlgorithm<bvec4> TextureGenerator::genGroundTexture(uint res)
{
	Palette palette;
	palette.setColor(randColor(), 0);
	palette.setColor(randColor(), 0.5);
	palette.setColor(randColor(), 1);

	auto bmpconvert = [](float x) { return bvec3(x * 255, x * 255, x * 255); };
	FractalNoise<SimplexNoise2D>(6, SimplexNoiseInstancer<SimplexNoise2D>(2, 2, _seed + 3249875)).
		generate({ res,res }).exportBMP("noise.bmp", palette);

	return ImageAlgorithm<bvec4>();
}

tim::bvec4 TextureGenerator::randColor()
{
	return { (byte)(_randEngine() % 256), (byte)(_randEngine() % 256) , (byte)(_randEngine() % 256), 1 };
}