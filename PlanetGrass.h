#pragma once

#include "math/Vector.h"
#include "Planet.h"

class PlanetGrass
{
public:
	PlanetGrass(Planet&, int seed = 42);
	
	void cull(const tim::Camera&, eastl::vector<ObjectInstance>&);

private:
	int _seed;
	Planet& _planet;

	enum { SIDE_X = 0, SIDE_NX, SIDE_Y, SIDE_NY, SIDE_Z, SIDE_NZ, NB_SIDE = 6 };
	bool _isSideReady[NB_SIDE] = { false };

	struct Batch
	{
		vec3 start, base1, base2;
		eastl::vector<eastl::pair<vec3, vec3>> vertex_normal;
		MeshBuffers mesh;
		Sphere sphere;
	};

	MeshBuffers _grassMesh[NB_SIDE];
	static constexpr int NB_SPLIT = 16;
	eastl::array<Batch, NB_SPLIT*NB_SPLIT> _batchSide[NB_SIDE];

private:
	void prepare();
	void generateMeshData(Planet&);
	void createMeshBuffers();
};