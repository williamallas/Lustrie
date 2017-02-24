#pragma once

#include <cmath>

namespace tim
{
    const float PI = 3.141592653589793238462643383279502884197169399375105820974944f;
	
    inline bool fcompare(float v1, float v2, float d) { return fabsf(v1-v2) <= d; }

    inline float toRad(float deg) { return deg*PI/180.f; }
    inline float toDeg(float rad) { return rad*180.f/PI; }
	
    inline uint uipow(uint x, uint p) { uint r=1; for(uint i=0 ; i<p ; i++)r*=x; return r; }

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
        return a*(1.f-x)+b*x;
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
