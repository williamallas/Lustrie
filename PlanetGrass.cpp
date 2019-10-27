#include "PlanetGrass.h"
#include <core/Logger.h>
#include "math/Frustum.h"

PlanetGrass::PlanetGrass(Planet& planet, int seed) : _seed(seed), _planet(planet)
{
	g_threadPool.push([=, &planet](int) {
		prepare();
		generateMeshData(planet);
		createMeshBuffers();
	});
}

void PlanetGrass::cull(const tim::Camera& camera, eastl::vector<ObjectInstance>& meshs)
{
	Frustum frust;
	tim::Camera cam = camera;
	frust.buildCameraFrustum(cam, Frustum::NEAR_PLAN);

	mat4 transform = mat4::Translation(_planet.position());

	for (int i = 0; i < NB_SIDE; ++i)
	{
		if (!_isSideReady[i])
			continue;

		for (Batch& batch : _batchSide[i])
		{
			if(frust.collide(batch.sphere) && (batch.sphere.center() - cam.pos).length2() < 50*50 && batch.mesh.numIndices() > 0)
				meshs.push_back({ &(batch.mesh), transform, MaterialParameter() });
		}	
	}
}

void PlanetGrass::prepare()
{
	for (int side = 0; side < NB_SIDE; ++side)
	{
		vec3 start, dir1, dir2;
		switch (side)
		{
		case SIDE_X:
			start = vec3(1, -1, 1), dir1 = vec3(0, 1, 0); dir2 = vec3(0, 0, -1); break;
		case SIDE_NX:
			start = vec3(-1, 1, 1), dir1 = vec3(0, -1, 0); dir2 = vec3(0, 0, -1); break;
		case SIDE_Y:
			start = vec3(1, 1, 1), dir1 = vec3(-1, 0, 0); dir2 = vec3(0, 0, -1); break;
		case SIDE_NY:
			start = vec3(-1, -1, 1), dir1 = vec3(1, 0, 0); dir2 = vec3(0, 0, -1); break;
		case SIDE_Z:
			start = vec3(-1, 1, 1), dir1 = vec3(1, 0, 0); dir2 = vec3(0, -1, 0); break;
		case SIDE_NZ:
			start = vec3(-1, -1, -1), dir1 = vec3(1, 0, 0); dir2 = vec3(0, 1, 0); break;
		}

		vec3 step1 = (dir1*2) / float(NB_SPLIT), step2 = (dir2*2) / float(NB_SPLIT);

		for (uint i = 0; i < NB_SPLIT; ++i)
		{
			for (uint j = 0; j < NB_SPLIT; ++j)
			{
				_batchSide[side][i*NB_SPLIT + j].base1 = step1;
				_batchSide[side][i*NB_SPLIT + j].base2 = step2;
				_batchSide[side][i*NB_SPLIT + j].start = start + (step1 * float(i)) + (step2 * float(j));
			}
		}
	}
}

void PlanetGrass::generateMeshData(Planet& planet)
{
	std::mt19937 randEngine(_seed);
	std::uniform_real_distribution<float> random;

	for (auto& side : _batchSide)
	{
		for (Batch& batch : side)
		{
			vec3 minB = { 9999,9999,9999 }, maxB = -minB;

			vec3 precomputedNormal[2][2];
			precomputedNormal[0][0] = planet.evalNormal(batch.start);
			precomputedNormal[1][0] = planet.evalNormal(batch.start + batch.base1);
			precomputedNormal[0][1] = planet.evalNormal(batch.start + batch.base2);
			precomputedNormal[1][1] = planet.evalNormal(batch.start + batch.base1 + batch.base2);

			vec3 v1 = batch.start.resized(planet.parameter().sizePlanet.x());
			vec3 v2 = (batch.start + batch.base1).resized(planet.parameter().sizePlanet.x());
			vec3 v3 = (batch.start + batch.base2).resized(planet.parameter().sizePlanet.x());
 
			float surface = (v1-v2).length() * (v1 - v3).length();
#ifdef _DEBUG
			int nbGrass = int(surface * 20 + 0.5f);
#else
			int nbGrass = int(surface * 50 + 0.5f);
#endif

			for (int i = 0; i < nbGrass; ++i)
			{
				vec2 r_vec(random(randEngine), random(randEngine));
				vec3 v = batch.start + batch.base1*r_vec.x() + batch.base2*r_vec.y();
				vec3 v_n = -v.normalized();

				float floorH = planet.isFloor(v);
				if (random(randEngine) < floorH)
					continue;

				v = planet.evalNoise(v);
				batch.vertex_normal.push_back(eastl::make_pair(v, 
					interpolateCos2(precomputedNormal[0][0], precomputedNormal[1][0], precomputedNormal[0][1], precomputedNormal[1][1], r_vec.x(), r_vec.y())));

				for (int a = 0; a < 3; ++a) minB[a] = min(minB[a], v[a]);
				for (int a = 0; a < 3; ++a) maxB[a] = max(maxB[a], v[a]);
			}

			batch.sphere = Sphere((minB + maxB)*0.5f, (minB - maxB).length());
		}
	}
}

void PlanetGrass::createMeshBuffers()
{
	int sideIndex = 0;
	for (auto& side : _batchSide)
	{
		eastl::array<uint, NB_SPLIT*NB_SPLIT> startIndexOffset;
		eastl::array<uint, NB_SPLIT*NB_SPLIT> nbIndex;
		Mesh sideMesh;

		int indexBatch = 0;
		uint indexAcc = 0;
		for (auto& batch : side)
		{
			startIndexOffset[indexBatch] = indexAcc;
			nbIndex[indexBatch] = batch.vertex_normal.size();

			for (auto v_n : batch.vertex_normal)
			{
				sideMesh.addFace({ { sideMesh.nbVertices(), 0,0,0 }, 1 });
				sideMesh.addVertexAndNormal(v_n.first, v_n.second);
			}

			indexAcc += batch.vertex_normal.size();
			++indexBatch;
			batch.vertex_normal.clear();
		}

		LOG("Planet side, # of grass:", sideMesh.nbVertices());
		_grassMesh[sideIndex] = MeshBuffers::createFromMesh(sideMesh, nullptr, 1, true, false);
		_grassMesh[sideIndex].setTopology(MeshBuffers::Points);

		indexBatch = 0;
		for (auto& batch : side)
		{
			batch.mesh = _grassMesh[sideIndex];
			batch.mesh.setNumIndices(nbIndex[indexBatch]);
			batch.mesh.setOffset(startIndexOffset[indexBatch]);
			++indexBatch;
		}

		_isSideReady[sideIndex] = true;
		++sideIndex;
	}
}