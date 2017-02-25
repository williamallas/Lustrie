#include "PerlinNoise.h"
#include <random>

namespace tim
{

PerlinNoise::PerlinNoise()
{
}

PerlinNoise::PerlinNoise(uint numLayer, uint firstLayerSize, int seed) : _layers(numLayer)
{
    for(uint i=0 ; i<numLayer ; ++i)
        _layers[i] = genNoise(firstLayerSize << i, seed);
}


PerlinNoise::~PerlinNoise()
{
}

ImageAlgorithm<float> PerlinNoise::generate(uivec2 res) const
{
    ImageAlgorithm<float> img(res);
    vec2 delta = vec2(1.f / res.x(),1.f / res.y());

    for(uint i=0 ; i<res.x() ; ++i)
    {
        for(uint j=0 ; j<res.y() ; ++j)
        {
            float val=0;
            float coef=0.5;
            for(size_t l=0 ; l < _layers.size() ; ++l)
            {
                val += _layers[l].getSmooth(vec2(delta.x()*i*_layers[l].size().x(), delta.y()*j*_layers[l].size().y())) * coef;
                coef *= 0.5;
            }

            img.set(i,j, val);
        }
    }

    return img;
}

ImageAlgorithm<float> PerlinNoise::genNoise(uint resolution, int seed, vec2 boundary)
{
    std::mt19937 randEngine(seed);
    std::uniform_real_distribution<float> random(boundary.x(),boundary.y());

    ImageAlgorithm<float> img({resolution, resolution});
    for(uint i=0 ; i<resolution ; ++i)
    {
        for(uint j=0 ; j<resolution ; ++j)
        {
            img.set(i,j, random(randEngine));
        }
    }

    return img;
}

}
