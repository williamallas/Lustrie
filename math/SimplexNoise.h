#pragma once

#include <EASTL/array.h>
#include "Vector.h"
#include "core/ImageAlgorithm.h"

namespace tim
{
    namespace internal
    {
        struct SimplexNoiseBaseBase
        {
            static int fastfloor(float x) { return x>0 ? (int)x : (int)x-1; }
            static eastl::array<vec3, 12> grad3;
        };

        template<class Point>
        class SimplexNoiseBase : protected SimplexNoiseBaseBase
        {
        public:
            SimplexNoiseBase(Point scale, int seed = 42) : _scale(scale)
            {
                for(int i=0 ; i<256 ; ++i)
                    _perm[i] = i;

                eastl::shuffle(_perm.begin(), _perm.begin()+256, std::mt19937(seed));
                eastl::copy(_perm.begin(), _perm.begin()+256, _perm.begin()+256);
            }

            SimplexNoiseBase(const SimplexNoiseBase&) = default;
            SimplexNoiseBase& operator=(const SimplexNoiseBase&) = default;
        protected:
            eastl::array<int, 512> _perm; // setup at the initialization
            Point _scale;
        };
    }

    class SimplexNoise3D : public internal::SimplexNoiseBase<vec3>
    {
    public:
        using Point = vec3;
        using SimplexNoiseBase::SimplexNoiseBase;

        float noise(Point) const;
    };

    class SimplexNoise2D : public internal::SimplexNoiseBase<vec2>
    {
    public:
        using Point = vec2;
        using SimplexNoiseBase::SimplexNoiseBase;

        float noise(Point) const;
        ImageAlgorithm<float> generate(uivec2 res) const;
    };

    template <class SimplexNoise>
    struct SimplexNoiseInstancer
    {
        typename SimplexNoise::Point _initial, _coef;
        int _seed;

        SimplexNoiseInstancer(typename SimplexNoise::Point initial = 1, typename SimplexNoise::Point coef = 2, int seed=42) : _initial(initial), _coef(coef), _seed(seed) {}

        SimplexNoise operator()(uint layer) const
        {
            typename SimplexNoise::Point scale = _initial * typename SimplexNoise::Point(_coef).apply([layer](float x) { return uipow(x, layer); });
            return SimplexNoise(scale, _seed+int(layer));
        }

    };

}
