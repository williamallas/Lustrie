#include "TextureGenerator.h"

using namespace tim;

eastl::vector<eastl::unique_ptr<FractalNoise<WorleyNoise<vec2>>>> g_fractalWorley[4];

namespace
{
	bvec4 toColb(const vec4& col) { return { (byte)(col[0]*255.f + 0.5f), (byte)(col[1] *255.f + 0.5f), (byte)(col[2] *255.f + 0.5f), (byte)(col[3] * 255.f + 0.5f) }; }
};

ImageAlgorithm<bvec4> TextureGenerator::genGroundTexture(uint res, const Palette& palette)
{
	auto bmpconvert = [](float x) { return bvec3(x * 255, x * 255, x * 255); };
	auto ridge = [](float x) { return fabsf((x - 0.5f) * 2); };

	if (_randEngine() % 2 == 0)
	{
		auto img = g_fractalWorley[3][_randEngine() % g_fractalWorley[3].size()]-> 
			generate({ res,res }, ridge).map([](float x) { return x; });

		return img.makeTilable().map(palette);
	}
	else
	{
		auto img = FractalNoise<SimplexNoise2D>(6, SimplexNoiseInstancer<SimplexNoise2D>(float(8 << (_randEngine() % 2)), 2, _seed + 3249875)).
			generate({ res,res }, ridge).map([](float x) { return x; });

		return img.makeTilable().map(palette);
	}
}

tim::ImageAlgorithm<tim::bvec4> TextureGenerator::genGrassTexture(tim::uint res, const Palette& palette)
{
	FractalNoise<SimplexNoise2D> noise(4, SimplexNoiseInstancer<SimplexNoise2D>(4, 2, _seed += 439354));
	tim::ImageAlgorithm<float> baseImg = noise.generate(res);
	for (uint i = 0; i < baseImg.size().x(); ++i) for (uint j = 0; j < baseImg.size().y(); ++j)
	{
		baseImg.set(i, j, baseImg.get(0, j));
	}

	return baseImg.map(palette);
}

tim::ImageAlgorithm<tim::bvec4> TextureGenerator::genTreeBarkTexture(tim::uint res, const tim::Palette& palette)
{
	FractalNoise<SimplexNoise2D> noise(5, SimplexNoiseInstancer<SimplexNoise2D>(4, 2, _seed += 4354));

	float nbSeuil = 6.f;
	auto bmpconvert = [](float x) { return bvec3(x * 255, x * 255, x * 255); };
	auto seuil = [=](float x) { return (int(x*nbSeuil)%3>0) ? float(int(x*nbSeuil)) / (nbSeuil-1) : x; };

	return noise.generate({ res, res }).map(seuil).map(palette);
}

tim::ImageAlgorithm<tim::bvec4> TextureGenerator::genLeafTexture(tim::uint res, const tim::Palette& palette)
{
	int index = _randEngine() % 2;
	return g_fractalWorley[index][_randEngine() % g_fractalWorley[index].size()]->generate({ res,res }).map(palette);
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

void TextureGenerator::genWorleyNoise()
{
	using T = FractalNoise<WorleyNoise<vec2>>;
	eastl::vector<std::future<void>> sync;
	int seed = (int)time(NULL);
	for (int i = 0; i < 4; ++i)
	{
		g_fractalWorley[i].clear();
		g_fractalWorley[i].resize(4);
		for (auto& fractal : g_fractalWorley[i])
		{
			sync.push_back(g_threadPool.push([&](int) {
				auto gen = eastl::make_unique<T>( 4, WorleyNoiseInstancer<WorleyNoise<vec2>>(25 << i, 4, 1, seed++) );
				eastl::swap(gen, fractal);
			}));
		}
	}

	for (auto& fut : sync)
		fut.wait();
}

eastl::vector<tim::ImageAlgorithm<tim::bvec4>> TextureGenerator::generateMips(const ImageAlgorithm<bvec4>& img, uint nbMips)
{
	if(nbMips == 0)
		nbMips = log2_ui(img.size().x()) - 1;

	eastl::vector<tim::ImageAlgorithm<tim::bvec4>> result;

	auto fimg = img.map([](bvec4 color) { return vec4(color[0], color[1], color[2], color[3]); });

	for (uint i = 0; i < nbMips; ++i)
	{
		fimg = fimg.resized(fimg.size() / 2);
		result.push_back(fimg.map([](vec4 color){ return bvec4(byte(color[0]), byte(color[1]), byte(color[2]), byte(color[3])); }));
	}

	return result;
}

vec4 TextureGenerator::getColorFromBank(ColorBank col)
{
	vec3 select = { _random(_randEngine), _random(_randEngine), _random(_randEngine) };
	eastl::vector<vec3> BANK;

	switch (col)
	{
	case BROWN: 
		BANK = { vec3(255, 84, 0) / 255.f, vec3(255, 123, 0) / 255.f, vec3(255, 124, 68) / 255.f, vec3(255, 146, 79) / 255.f }; break;
	case ORANGE:
		BANK = { vec3(253, 247, 71) / 255.f, vec3(255, 157, 0) / 255.f, vec3(255, 100, 0) / 255.f, vec3(255, 43, 28) / 255.f }; break;
	case ORANGE2:
		BANK = { vec3(255, 222, 94) / 255.f, vec3(255, 191, 0) / 255.f, vec3(255, 138, 71) / 255.f, vec3(255, 80, 0) / 255.f }; break;
	case GREEN:
		BANK = { vec3(52, 255, 38) / 255.f, vec3(114, 255, 20) / 255.f, vec3(133,255,85) / 255.f, vec3(187, 255, 91) / 255.f }; break;
	case GREEN2:
		BANK = { vec3(204,255,0) / 255.f, vec3(178, 255, 0) / 255.f, vec3(97,255,0) / 255.f, vec3(0,1,0) }; break;
	case WHITE:
		BANK = { vec3(1,1,1), vec3(195/255.f, 1, 1), vec3(195 / 255.f,195 / 255.f,1) }; break;
	case PURPLE:
		BANK = { vec3(255, 163, 227) / 255.f, vec3(255, 86, 218) / 255.f, vec3(230, 48, 255) / 255.f, vec3(60, 5, 255) / 255.f }; break;
	case PURPLE2:
		BANK = { vec3(255, 163, 253) / 255.f, vec3(255, 43, 152) / 255.f, vec3(255, 30, 80) / 255.f, vec3(255, 5, 45) / 255.f }; break;
	}

	size_t index1 = std::min(size_t(BANK.size() * select.x()), BANK.size() - 1);
	size_t index2 = std::min(size_t(BANK.size() * select.y()), BANK.size() - 1);

	return vec4(interpolate(BANK[index1], BANK[index2], select.z()), 1);
}

Palette TextureGenerator::genPalette(const eastl::vector<ColorBank>& choices, uivec2 nbColorsRange)
{
	std::uniform_int_distribution<uint> randInt(nbColorsRange.x(), nbColorsRange.y());
	Palette palette;
	uint nbColor = randInt(_randEngine);

	if (nbColor == 0)
		nbColor++;

	for (uint i = 0; i < nbColor; ++i)
	{
		vec4 col = getColorFromBank(choices[_randEngine() % choices.size()]);// * (0.4f + 0.6f * (1.f-powf(_random(_randEngine), 2.f)));
		if(_randEngine()%2 == 0)
			col *= (0.3f + 0.7f * (1.f - powf(_random(_randEngine), 2.f)));

		palette.setColor(toColb(col), float(i) / (nbColor - 1));
	}

	return palette;
}