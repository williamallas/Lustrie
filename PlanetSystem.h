#pragma once

#include "PlanetGrass.h"
#include "PlanetPlants.h"

class PlanetSystem
{
public:
	static constexpr int TEX_PLANET = 0;
	static constexpr int NB_TEX_PLANET = 1;

	static constexpr int TEX_PLANET_GRASS = TEX_PLANET + NB_TEX_PLANET;

	static constexpr int TEX_PLANET_PLANT_TRUNK = TEX_PLANET_GRASS+1;
	static constexpr int NB_TEX_PLANET_PLANT_TRUNK = 1;

	static constexpr int TEX_PLANET_PLANT_LEAF = TEX_PLANET_PLANT_TRUNK + NB_TEX_PLANET_PLANT_TRUNK;
	static constexpr int NB_TEX_PLANET_PLANT_LEAF = 1;

	eastl::unique_ptr<Material> planetMaterial;
	eastl::vector<eastl::shared_ptr<Material>> grassMaterial;

	eastl::unique_ptr<Planet> planet;
	eastl::vector<eastl::unique_ptr<PlanetGrass>> grassOnPlanet;

	eastl::unique_ptr<Material> plantMaterial;
	eastl::unique_ptr<Material> leafMaterial;
	eastl::unique_ptr<PlanetPlants> plants;
};