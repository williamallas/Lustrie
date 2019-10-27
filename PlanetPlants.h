#pragma once

#include "Planet.h"
#include "graphics/RendererStruct.h"

class PlanetPlants
{
public:
	PlanetPlants(int seed);
	
	void cull(const tim::Camera&, eastl::vector<ObjectInstance>&, eastl::vector<ObjectInstance>&);

	void createTree(int sizeCategorie, int number);
	void populatePlant(Planet&, size_t plantIndex, int nbInstance);

private:
	int _seed;

	struct Plant
	{
		struct Lod
		{
			Lod(const Lod&) = default;
			Lod& operator=(const Lod&) = default;

			eastl::shared_ptr<MeshBuffers> meshs[2];
			float distance;
		};
		eastl::vector<Lod> lods;
		Sphere boundingSphere;
	};

	std::mutex _treeVectorMutex;
	eastl::vector<Plant> _plants;

	struct Instance
	{
		tim::mat4 transform;
		MaterialParameter material[2];
		size_t indexPlant;
	};

	eastl::vector<Instance> _instances;
};