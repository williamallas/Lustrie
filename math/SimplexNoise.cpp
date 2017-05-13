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
     return 70.0 * N.dot(1);
}

float SimplexNoise3D::noise(vec3 v) const
{
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
     return 32 * N.dot(vec4::construct(1));
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

