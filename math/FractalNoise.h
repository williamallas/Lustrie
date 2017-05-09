#pragma once

#include <EASTL/functional.h>
#include <EASTL/vector.h>
#include "core/type.h"
#include "core/ImageAlgorithm.h"

namespace tim
{
    template<class Noise>
    class FractalNoise
    {
    public:
        FractalNoise(int numLayer, eastl::function<Noise(uint)> instancer)
        {
            for(int i=0 ; i<numLayer ; ++i)
                _layers.push_back(instancer(i));
        }

        float noise(typename Noise::Point v) const
        {
            float res=0;
            float coef = 0.5f;
            for(size_t i=0 ; i<_layers.size() ; ++i)
            {
                res += _layers[i].noise(v) * coef;
                coef *= 0.5f;
            }

            return res;
        }

        ImageAlgorithm<float> generate(uivec2 res) const
        {
            ImageAlgorithm<float> img(res);
            vec2 delta = vec2(1.f / (res.x()-1),1.f / (res.y()-1));

            for(uint i=0 ; i<res.x() ; ++i)
                for(uint j=0 ; j<res.y() ; ++j)
                    img.set(i,j, noise(delta * vec2(i,j)));

            return img;
        }

    private:
        eastl::vector<Noise> _layers;
    };
}
