#include "PlanetPlants.h"
#include "geometry\LTree.h"
#include "geometry\LeafGenerator.h"
#include "PlanetSystem.h"

PlanetPlants::PlanetPlants(int seed) : _seed(seed)
{
	
}

void PlanetPlants::cull(const tim::Camera&, eastl::vector<ObjectInstance>& trunkPart, eastl::vector<ObjectInstance>& leafPart)
{
	for (auto& inst : _instances)
	{
		trunkPart.push_back({ _plants[inst.indexPlant].lods[0].meshs[0].get(), inst.transform, inst.material[0] });
		leafPart.push_back({ _plants[inst.indexPlant].lods[0].meshs[1].get(), inst.transform, inst.material[1] });
	}
}

namespace
{
	LTree::PredefinedTree randPredefFrom(int sizeCategorie, int r)
	{
		switch (sizeCategorie)
		{
		case 0:
			return (LTree::PredefinedTree)(LTree::TREE_1 + r%5);
		case 1:
			return (LTree::PredefinedTree)(LTree::TREE_1 + r % 5);
		case 2:
			return (LTree::PredefinedTree)(LTree::BIG_TREE1 + r % 4);
		default:
			return (LTree::PredefinedTree)(LTree::TREE_1 + r % 5);
		}
	}

	LTree::Parameter genRandParam(int seed, int sizeCategorie)
	{
		std::mt19937 randEngine(seed);
		std::uniform_real_distribution<float> random(0,1);

		auto p1 = LTree::getPredefinedTree(randPredefFrom(sizeCategorie, randEngine()));
		auto p2 = LTree::getPredefinedTree(randPredefFrom(sizeCategorie, randEngine()));

		return LTree::Parameter::interpolate(p1, p2, random(randEngine)).alterate(seed+=3459387, random(randEngine)*0.99f);
	}
}

void PlanetPlants::createTree(int sizeCategorie, int nb)
{
	auto baseParam = genRandParam(_seed++, sizeCategorie);
	LeafGenerator::Parameter leafParam = LeafGenerator::Parameter::gen(_seed++, sizeCategorie);

	for (int i = 0; i < nb; ++i)
	{
		LTree treeGenerator(baseParam.alterate(_seed++, 0.1f), _seed++);
		UVMesh tree = treeGenerator.generateUVMesh(8);
		tree.computeNormals(true);

		LeafGenerator genLeaf;
		genLeaf.generate(leafParam);
		//genLeaf.mesh().exportToObj("leaf.obj");

		LTree::LeafParameter genLeafParam;
		genLeafParam.leaf = genLeaf.mesh();
		genLeafParam.orientation = LTree::GaussPDF({ -0.6f,0.6f });
		genLeafParam.tilt = LTree::GaussPDF({ 0.0f,0.3f });
		genLeafParam.scale = LTree::GaussPDF({ 0.75f,1.5f });
		genLeafParam.density = sizeCategorie == 1 ? 6:2;
		genLeafParam.depth = 2;

		MeshBuffers treeBuffer = MeshBuffers::createFromMesh(tree);
		MeshBuffers leafTreeBuffer = MeshBuffers::createFromMesh(treeGenerator.generateLeaf(genLeafParam));

		Plant plant;
		plant.lods.push_back({ { eastl::make_unique<MeshBuffers>(treeBuffer), eastl::make_unique<MeshBuffers>(leafTreeBuffer) }, 0 });
		plant.boundingSphere = tree.computeBoundingSphere();

		std::lock_guard<std::mutex> _(_treeVectorMutex);
		_plants.push_back(plant);
	}
}

void PlanetPlants::populatePlant(Planet& planet, size_t plantIndex, int nbPlants)
{
	_ASSERT(plantIndex < _plants.size());

	std::mt19937 randEngine(_seed++);
	std::uniform_real_distribution<float> random;

	for (int i = 0; i < nbPlants; ++i)
	{
		float u = (random(randEngine) - 0.5f) * 2;
		float theta = random(randEngine) * 2 * PI;
		float tmp = sqrtf(1.f - u*u);
		vec3 pos(tmp * cosf(theta), tmp * sinf(theta), u);

		vec3 translation = planet.evalNoise(pos);

		vec3 norm = -planet.evalNormal(pos);
		norm = interpolate(pos.normalized(), norm, random(randEngine));

		Instance inst;
		inst.transform = mat4::constructTransformation(mat3::changeBasis(norm)*mat3::RotationZ(random(randEngine)*PI * 2), 
													   translation - norm*0.2f, vec3::construct(0.8f+random(randEngine)*0.4f)).transposed();
		inst.indexPlant = plantIndex;
		inst.material[0].textures[0] = PlanetSystem::TEX_PLANET_PLANT_TRUNK + randEngine() % PlanetSystem::NB_TEX_PLANET_PLANT_TRUNK;
		inst.material[1].textures[0] = PlanetSystem::TEX_PLANET_PLANT_LEAF + randEngine() % PlanetSystem::NB_TEX_PLANET_PLANT_LEAF;
		_instances.push_back(inst);
	}
}