#pragma once

#include <cmath>
#include "extern/rtnorm.hpp"

namespace tim
{
    template<typename T> T getPI() { return T(3.141592653589793238462643383279502884197169399375105820974944); }
    template<typename T> T getTAU() { return getPI<T>() * 2; }

    const float PI  = getPI<float>();
    const float TAU = getTAU<float>();
	
    inline bool fcompare(float v1, float v2, float d) { return fabsf(v1-v2) <= d; }

    inline float toRad(float deg) { return deg*PI/180.f; }
    inline float toDeg(float rad) { return rad*180.f/PI; }
	
    template<typename T>
    inline T uipow(T x, uint p) { T r(1); for(uint i=0 ; i<p ; i++)r*=x; return r; }

    inline float zero(float x) { if(fcompare(x,0,1e-16f)) return 0.f; else return x; }

    inline float sigma(float x, float strength=2) { return 0.5f+0.5f*tanhf((x-0.5f)*strength); }

    inline float activate(float x, float center, float width)
    {
        x=(x-center)/width;
        x = (x > 1?1 : (x < -1?-1 : x));
        return x*0.5f+0.5f;
    }

    template <class T>
    T pmod(T x, T m)
    {
        T r = x%m;
        return r<0 ? r+m : r;
    }

    template <class T>
    T log2_ui(T x)
    {
        if (x==0) return 0;
        T val = 0;
        while (x)
        {
            ++val;
            x >>= 1;
        }
        return val-1;
    }

    template <class T>
    T le_power2(T n)
    {
        T p = 1;
        while (p <= n) p <<= 1;
        return p >> 1;
    }

    template <class T>
    T l_power2(T n)
    {
        T p = 1;
        while (p < n) p <<= 1;
        return p >> 1;
    }

    template <class T>
    T ge_power2(T n)
    {
        T p = 1;
        while (p < n) p <<= 1;
        return p;
    }

    template <class T>
    T g_power2(T n)
    {
        T p = 1;
        while (p <= n) p <<= 1;
        return p;
    }
	

    template<class T>
    inline T interpolate(T a, T b, float x)
    {
        return (T)(a*(1.f-x) + b*x);
    }

    template<class T>
    inline T interpolateCos(T a, T b, float x)
    {
        return interpolate(a,b, (1.f-cosf(x*PI))*0.5f);
    }

    template<class T>
    inline T interpolate2(T a, T b, T c, T d, float x, float y)
    {
        return interpolate(interpolate(a,b, x), interpolate(c,d, x), y);
    }

    template<class T>
    inline T interpolateCos2(T a, T b, T c, T d, float x, float y)
    {
        return interpolateCos(interpolateCos(a,b, x), interpolateCos(c,d, x), y);
    }
}
