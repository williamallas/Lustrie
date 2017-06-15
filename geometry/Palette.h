#pragma once

#include <EASTL/array.h>
#include "math/Vector.h"
#undef min
#undef max

namespace tim
{
    class Palette
    {
    public:
        Palette() { setColor(bvec4(0,0,0,1), 0).setColor(bvec4(1,1,1,1), 1).complete(); }
        Palette(const Palette&) = default;

        Palette(bvec4 first, bvec4 last)
        { setColor(first, 0).setColor(last, 1).complete(); }

        Palette& operator=(const Palette&) = default;

        Palette& setColor(bvec4 c, float x)
        {
            x = std::min(std::max(x, 0.f), 1.f);
            int i = int(x*(RESOLUTION-1) + 0.5f);
            _colors[i] = c; _filled[i] = true;
            complete();
            return *this;
        }

        void clear() { *this = Palette(); }

        bvec4 operator()(float x) const
        {
            x = std::min(std::max(x, 0.f), 1.f);
            int i = int(x*(RESOLUTION-1) + 0.5f);
            return _colors[i];
        }

    private:
        static const int RESOLUTION = 1024;
        eastl::array<bvec4, RESOLUTION> _colors;
        eastl::array<bool, RESOLUTION> _filled = {{false}};

        static vec4 tovec4(bvec4 c) { return vec4(c[0] / 255.f, c[1] / 255.f, c[2] / 255.f, c[3] / 255.f); }
        static bvec4 tobvec4(vec4 v) { return bvec4(byte(v[0] * 255.f + 0.5f), byte(v[1] * 255.f + 0.5f), byte(v[2] * 255.f + 0.5f), byte(v[3] * 255.f + 0.5f)); }

        void complete()
        {
            int lastIndex = 0, nextIndex=0;
            for(int i=0 ; i<RESOLUTION ; ++i)
            {
                if(_filled[i]) // find next filled
                {
                    lastIndex = i;
                    nextIndex = RESOLUTION-1;
                    for(int j=i+1 ; j<RESOLUTION ; ++j)
                    {
                        if(_filled[j])
                        {
                            nextIndex=j;
                            break;
                        }
                    }
                }
                else
                {
                    float x = (i-lastIndex) / float(nextIndex-lastIndex);
                    _colors[i] = tobvec4(interpolate(tovec4(_colors[lastIndex]), tovec4(_colors[nextIndex]), x));
                }
            }
        }
    };
}
