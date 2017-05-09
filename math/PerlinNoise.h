#pragma once

#include "core\ImageAlgorithm.h"

namespace tim
{

    class PerlinNoise
    {
    public:
        PerlinNoise(uint numLayer, uint firstLayerSize, int seed=42);
        ~PerlinNoise();

        PerlinNoise(const PerlinNoise&) = default;
        PerlinNoise& operator=(const PerlinNoise&) = default;

        ImageAlgorithm<float> generate(uivec2) const;

    private:
        eastl::vector<ImageAlgorithm<float>> _layers;

    private:
        static ImageAlgorithm<float> genNoise(uint, int seed, vec2 boundary = vec2(0,1));
    };

}

