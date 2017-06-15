#pragma once

#include <EASTL/vector.h>
#include "geometry/Mesh.h"
#include "math/Sphere.h"
#include "math/Camera.h"
#include "graphics\Graphics.h"

#include <math/FractalNoise.h>
#include <math/SimplexNoise.h>
#include <math/WorleyNoise.h>

#include <core/ctpl_stl.h>
extern ctpl::thread_pool g_threadPool;

class Planet : NonCopyable
{
public:
	struct Parameter
	{
		vec2 sizePlanet = { 100, 25 };
		float largeRidgeCoef = 1;
		float largeRidgeScale = 1;
		float largeExponent = 1;

		float largeSimplexCoef = 1;
		float largeSimplexScale = 0;

		float simplexDetailCoef = 15;
		float simplexDetailScale = 0.1f;

		float floorHeight = 0.25f;
		float largeCoefFloor = 0.02f, detailCoefFloor = 0.2f;
	};

	Planet(tim::uint, const Parameter& param = Parameter());

    template<class Noise> void applyNoise(const Noise&, float factor, int side = Planet::NB_SIDE);

    tim::UVMesh generateMesh(tim::vec3) const;

	void cull(const tim::Camera&, eastl::vector<MeshBuffers*>&);

	tim::vec3 computeUp(tim::vec3 pos);

private:
	vec3 _position;
	Parameter _parameter;

    using BatchInstance = eastl::vector<tim::BaseMesh::Face>;
	static const int NB_LODS = 4;

    enum { LOW_RES_PLANET = -1, SIDE_X=0, SIDE_NX, SIDE_Y, SIDE_NY, SIDE_Z, SIDE_NZ, NB_SIDE=6 };
    eastl::array<tim::BaseMesh, NB_SIDE> _planetSide;
	eastl::array<tim::BaseMesh, NB_SIDE> _planetSideLowRes;

    tim::uint _gridResolution;
    eastl::vector<tim::uint> _gridIndex;

	template <class T> using GridType = eastl::array<eastl::array<T, 8>, 8>;
	GridType< eastl::array<BatchInstance, NB_LODS> > _grid;

	bool _isLowResReady = false;
	bool _isSideReady[NB_SIDE] = { false };

	eastl::array<MeshBuffers, NB_SIDE> _lowResMesh;
	eastl::array< GridType<eastl::array<MeshBuffers, NB_LODS>>, NB_SIDE > _planetMesh;

private:
    tim::uint indexGrid(tim::uint,tim::uint) const;
    tim::uint& indexGrid(tim::uint,tim::uint);

    void generateGrid(tim::uint);
    void generateBatchIndex(tim::uint, bool);
	void generateLowResGrid(tim::uint);
	void generateSideMeshBuffers(int side);

	int distanceToLod(float) const;

private:

	struct NoiseClosure
	{
		static const float BASE_SIZE;

		NoiseClosure(int seedRand, const Parameter& param) : parameter(param), seed(seedRand), pFactor(param.sizePlanet.x() / BASE_SIZE) {}

		/* parameter of the planet */
		Parameter parameter;
		int seed;
		float pFactor;

		float ridgeFun(float x) { return 1.f - powf(fabsf(x * 2 - 1), parameter.largeExponent); };

		/* The noise generators */
		FractalNoise<SimplexNoise3D> noiseForRidge = FractalNoise<SimplexNoise3D>(3, SimplexNoiseInstancer<SimplexNoise3D>(parameter.largeRidgeCoef, 2, seed));
		FractalNoise<SimplexNoise3D> noiseForSimplex = FractalNoise<SimplexNoise3D>(3, SimplexNoiseInstancer<SimplexNoise3D>(parameter.largeSimplexCoef, 2, seed+1));
		FractalNoise<SimplexNoise3D> simplexForDetails = FractalNoise<SimplexNoise3D>(3, SimplexNoiseInstancer<SimplexNoise3D>(parameter.simplexDetailCoef, 2, seed+2));
		//FractalNoise<WorleyNoise<vec3>> worlNoise = FractalNoise<WorleyNoise<vec3>>(1, WorleyNoiseInstancer<WorleyNoise<vec3>>(5000, 5));

		/* The noise composition */
		float noiseFun(vec3 v) 
		{
			v *= pFactor;
			float large = parameter.largeRidgeScale * noiseForRidge.noise(v, [=](float x) { return ridgeFun(x); }) +
						  parameter.largeSimplexScale * noiseForSimplex.noise(v);
			large = powf(large, parameter.largeExponent);

			float detail = parameter.simplexDetailScale * simplexForDetails.noise(v);

			if (large < parameter.floorHeight)
			{
				large = parameter.floorHeight + large * parameter.largeCoefFloor;
				large += parameter.simplexDetailScale * parameter.detailCoefFloor;
				detail *= parameter.detailCoefFloor;
			}
				
			 return parameter.sizePlanet.x() + parameter.sizePlanet.y() * (large+detail);
		};

		eastl::function<float(vec3)> noiseFun()
		{
			return [&](vec3 v) { return noiseFun(v); };
		}
	};

	NoiseClosure _noise;
};

inline tim::uint Planet::indexGrid(tim::uint i,tim::uint j) const
{ return _gridIndex[j +  i*_gridResolution]; }

inline tim::uint& Planet::indexGrid(tim::uint i,tim::uint j)
{ return _gridIndex[j +  i*_gridResolution]; }

template<class Noise> void Planet::applyNoise(const Noise& noise, float factor, int side)
{
	if (side == Planet::NB_SIDE || side == Planet::LOW_RES_PLANET)
	{
		for (auto& side : _planetSideLowRes)
		{
			side.mapVertices([&](tim::vec3 v) {
				v.normalize();
				v *= (1 + noise(v*0.5f + 0.5f)*factor);
				return v;
			});
		}
	}

	for (size_t i=0  ; i<_planetSide.size() ; ++i)
	{
		if (side != i && side != Planet::NB_SIDE)
			continue;

		auto& side = _planetSide[i];
		side.mapVertices([&](tim::vec3 v) {
			v.normalize();
			v *= (1 + noise(v*0.5f + 0.5f)*factor);
			return v;
		});
	}
}
