#pragma once

#include "core/type.h"
#include "core/ImageAlgorithm.h"
#include <random>
#include <EASTL/sort.h>

namespace tim
{
    template<class T>
    class WorleyNoise
    {
    public:
        using Point = T;
        WorleyNoise(uint nbPoints, int nth = 1, int seed = 42);

        WorleyNoise(const WorleyNoise&) = default;
        WorleyNoise& operator=(const WorleyNoise&) = default;

        float noise(const T& x) const;

    private:
        int _nth;
        eastl::vector<T> _points;
    };

    template<class T>
    struct WorleyNoiseInstancer
    {
        uint _nbPoints, _layerCoef;
        int _nth, _seed;

        WorleyNoiseInstancer(uint nbPoints, uint layerCoef = 3, uint nth = 1, int seed = 42) : _nbPoints(nbPoints), _layerCoef(layerCoef), _nth(nth), _seed(seed) {}

        T operator()(uint layer) const
        {
            return T(_nbPoints * uipow(_layerCoef, layer), _nth, _seed+layer);
        }
    };

    /********************/
    /*** Implentation ***/
    /********************/

    /** Worley Noise **/

    template<class T> WorleyNoise<T>::WorleyNoise(uint nbPoints, int nth, int seed) : _nth(nth)
    {
        std::mt19937 randEngine(seed);
        std::uniform_real_distribution<float> random(0,1);

        for(uint i=0 ; i<nbPoints ; ++i)
        {
            T v;
            for(uint j=0 ; j<T::Length ; ++j)
                v[j] = random(randEngine);
            _points.push_back(v);
        }
    }

    template<class T> float WorleyNoise<T>::noise(const T& x) const
    {
        int nth = _nth>0 ? _nth:1;
        eastl::vector<float> dists(_points.size());
        for(size_t i=0 ; i<_points.size() ; ++i)
            dists[i] = (_points[i] - x).length2();

        eastl::partial_sort(dists.begin(), dists.begin()+nth, dists.end());

        return sqrt(dists[nth-1]);
    }
}
