#include "SimplexNoise.h"
#include <random>
#include <EASTL/algorithm.h>

namespace tim
{

namespace internal
{
    eastl::array<vec3, 12> SimplexNoiseBaseBase::grad3 =
        {{{1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
          {1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
          {0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}}};
}

float SimplexNoise2D::noise(vec2 v) const
{
    v *= _scale;
     // Skew the input space to determine which simplex cell we're in
     const float F2 = 0.5f*(sqrt(3.f)-1.f);
     float s = (v.x()+v.y())*F2; // Hairy factor for 2D

     int i = fastfloor(v.x()+s);
     int j = fastfloor(v.y()+s);

     const float G2 = (3.f-sqrt(3.f))/6.f;

     float t = (i+j)*G2;
     vec2 V[3];
     V[0] = v - vec2(i-t, j-t); // Unskew the cell origin back to (x,y,z) space

     // For the 2D case, the simplex shape is an equilateral triangle.
     // Determine which simplex we are in.
     int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
     if(V[0].x() > V[0].y()) {i1=1; j1=0;} // lower triangle, XY order: (0,0)->(1,0)->(1,1)
     else                    {i1=0; j1=1;} // upper triangle, YX order: (0,0)->(0,1)->(1,1)

     // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
     // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
     // c = (3-sqrt(3))/6
     V[1] = V[0] - vec2(i1, j1) + vec2(G2, G2);
     V[2] = V[0] - vec2(1,1) + vec2(2.f*G2, 2.f*G2);

     // Work out the hashed gradient indices of the three simplex corners
     int ii = i & 255;
     int jj = j & 255;
     int gi[3] = { _perm[ii+_perm[jj]] % 12,
                   _perm[ii+i1+_perm[jj+j1]] % 12,
                   _perm[ii+1+_perm[jj+1]] % 12 };


     // Calculate the contribution from the three corners
     vec3 T,N;
     for(int i=0 ; i<3 ; ++i)
     {
         T[i] = 0.5f - V[i].dot(V[i]);
         if(T[i] < 0)
             N[i] = 0;
         else
         {
            T[i] *= T[i];
            N[i] = T[i]*T[i] * vec2(grad3[gi[i]][0], grad3[gi[i]][1]).dot(V[i]);
         }
     }

     // Add contributions from each corner to get the final noise value.
     // The result is scaled to return values in the interval [-1,1].
     return 70.0f * N.dot(1);
}

// https://github.com/WardBenjamin/SimplexNoise/blob/master/SimplexNoise/Noise.cs
namespace
{
    float grad(int hash, float x, float y, float z)
    {
        int h = hash & 15;     // Convert low 4 bits of hash code into 12 simple
        float u = h < 8 ? x : y; // gradient directions, and compute dot product.
        float v = h < 4 ? y : h == 12 || h == 14 ? x : z; // Fix repeats at h = 12 to 15
        return ((h & 1) != 0 ? -u : u) + ((h & 2) != 0 ? -v : v);
    }
}
float SimplexNoise3D::noise(vec3 v) const
{
    v *= _scale;
    float x=v.x();
    float y=v.y();
    float z=v.z();
    // Simple skewing factors for the 3D case
    const float F3 = 0.333333333f;
    const float G3 = 0.166666667f;

    float n0, n1, n2, n3; // Noise contributions from the four corners

    // Skew the input space to determine which simplex cell we're in
    float s = (x + y + z) * F3; // Very nice and simple skew factor for 3D
    float xs = x + s;
    float ys = y + s;
    float zs = z + s;
    int i = fastfloor(xs);
    int j = fastfloor(ys);
    int k = fastfloor(zs);

    float t = (float)(i + j + k) * G3;
    float X0 = i - t; // Unskew the cell origin back to (x,y,z) space
    float Y0 = j - t;
    float Z0 = k - t;
    float x0 = x - X0; // The x,y,z distances from the cell origin
    float y0 = y - Y0;
    float z0 = z - Z0;

    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
    int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
    int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords

    /* This code would benefit from a backport from the GLSL version! */
    if (x0 >= y0)
    {
        if (y0 >= z0)
        { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0; } // X Y Z order
        else if (x0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; } // X Z Y order
        else { i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; } // Z X Y order
    }
    else
    { // x0<y0
        if (y0 < z0) { i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; } // Z Y X order
        else if (x0 < z0) { i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; } // Y Z X order
        else { i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; } // Y X Z order
    }

    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    // c = 1/6.

    float x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
    float y1 = y0 - j1 + G3;
    float z1 = z0 - k1 + G3;
    float x2 = x0 - i2 + 2.0f * G3; // Offsets for third corner in (x,y,z) coords
    float y2 = y0 - j2 + 2.0f * G3;
    float z2 = z0 - k2 + 2.0f * G3;
    float x3 = x0 - 1.0f + 3.0f * G3; // Offsets for last corner in (x,y,z) coords
    float y3 = y0 - 1.0f + 3.0f * G3;
    float z3 = z0 - 1.0f + 3.0f * G3;

    // Wrap the integer indices at 256, to avoid indexing perm[] out of bounds
    int ii = i&255;
    int jj = j&255;
    int kk = k&255;

    // Calculate the contribution from the four corners
    float t0 = 0.6f - x0 * x0 - y0 * y0 - z0 * z0;
    if (t0 < 0.0f) n0 = 0.0f;
    else
    {
        t0 *= t0;
        n0 = t0 * t0 * grad(_perm[ii + _perm[jj + _perm[kk]]], x0, y0, z0);
    }

    float t1 = 0.6f - x1 * x1 - y1 * y1 - z1 * z1;
    if (t1 < 0.0f) n1 = 0.0f;
    else
    {
        t1 *= t1;
        n1 = t1 * t1 * grad(_perm[ii + i1 + _perm[jj + j1 + _perm[kk + k1]]], x1, y1, z1);
    }

    float t2 = 0.6f - x2 * x2 - y2 * y2 - z2 * z2;
    if (t2 < 0.0f) n2 = 0.0f;
    else
    {
        t2 *= t2;
        n2 = t2 * t2 * grad(_perm[ii + i2 + _perm[jj + j2 + _perm[kk + k2]]], x2, y2, z2);
    }

    float t3 = 0.6f - x3 * x3 - y3 * y3 - z3 * z3;
    if (t3 < 0.0f) n3 = 0.0f;
    else
    {
        t3 *= t3;
        n3 = t3 * t3 * grad(_perm[ii + 1 + _perm[jj + 1 + _perm[kk + 1]]], x3, y3, z3);
    }

    // Add contributions from each corner to get the final noise value.
    // The result is scaled to stay just inside [-1,1]
    return 32.0f * (n0 + n1 + n2 + n3); // TODO: The scale factor is preliminary!

    /*

    v *= _scale;
    //double n0, n1, n2, n3; // Noise contributions from the four corners

    // Skew the input space to determine which simplex cell we're in
    static const float F3 = 1.f/3.f;
    float s = (v[0]+v[1]+v[2])*F3; // Very nice and simple skew factor for 3D

     int i = fastfloor(v[0]+s);
     int j = fastfloor(v[1]+s);
     int k = fastfloor(v[2]+s);

     static const float G3 = 1.f/6.f; // Very nice and simple unskew factor, too

     float t = (i+j+k)*G3;

     vec3 V[4];
     V[0] = v - vec3(i-t, j-t, k-t); // Unskew the cell origin back to (x,y,z) space

     // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
     // Determine which simplex we are in.
     int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
     int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords
     if(V[0].x() >= V[0].y())
     {
        if(V[0].y() >= V[0].z())      { i1=1; j1=0; k1=0; i2=1; j2=1; k2=0; } // X Y Z order
        else if(V[0].x() >= V[0].z()) { i1=1; j1=0; k1=0; i2=1; j2=0; k2=1; } // X Z Y order
        else                      { i1=0; j1=0; k1=1; i2=1; j2=0; k2=1; } // Z X Y order
     }
     else // x<y
     {
        if(V[0].y() < V[0].z())      { i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; } // Z Y X order
        else if(V[0].x() < V[0].z()) { i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; } // Y Z X order
        else                     { i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; } // Y X Z order
     }

     // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
     // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
     // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
     // c = 1/6.

     V[1] = V[0] - vec3(i1, j1, k1) + vec3::construct(G3);       // Offsets for first corner in (x,y,z) coords
     V[2] = V[0] - vec3(i2, j2, k2) + vec3::construct(2.f * G3); // Offsets for third corner in (x,y,z) coords
     V[3] = V[0] - vec3::construct(1) + vec3::construct(3.f*G3); // Offsets for last corner in (x,y,z) coords

     // Work out the hashed gradient indices of the four simplex corners
     int ii = i & 255;
     int jj = j & 255;
     int kk = k & 255;

     int gi[4] = { _perm[ii+_perm[jj+_perm[kk]]] % 12,
                   _perm[ii+i1+_perm[jj+j1+_perm[kk+k1]]] % 12,
                   _perm[ii+i2+_perm[jj+j2+_perm[kk+k2]]] % 12,
                   _perm[ii+1+_perm[jj+1+_perm[kk+1]]] % 12 };

     // Calculate the contribution from the four corners
     vec4 T,N;
     for(int i=0 ; i<4 ; ++i)
     {
         T[i] = 0.5f - V[i].dot(V[i]);
         if(T[i] < 0)
             N[i] = 0;
         else
         {
            T[i] *= T[i];
            N[i] = T[i]*T[i] * grad3[gi[i]].dot(V[i]);
         }
     }

     // Add contributions from each corner to get the final noise value.
     // The result is scaled to stay just inside [-1,1]
     //std::cout << 32*N.dot(vec4::construct(1)) << std::endl;
     return 32 * N.dot(vec4::construct(1));

     */
}

ImageAlgorithm<float> SimplexNoise2D::generate(uivec2 res) const
{
    ImageAlgorithm<float> img(res);
    vec2 delta = vec2(1.f / res.x(),1.f / res.y());

    for(uint i=0 ; i<res.x() ; ++i)
        for(uint j=0 ; j<res.y() ; ++j)
            img.set(i,j, noise(delta * vec2(i,j)));

    return img;
}

}

