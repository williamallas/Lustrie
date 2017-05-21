#pragma once

#include <EASTL/vector.h>
#include "geometry/Mesh.h"
#include "math/Sphere.h"

class Planet
{
public:
    Planet(tim::uint);

    template<class Noise> void applyNoise(const Noise&, float factor);

    tim::UVMesh generateMesh(tim::vec3) const;

private:

    using BatchInstance = eastl::vector<tim::BaseMesh::Face>;

    enum { SIDE_X=0, SIDE_NX, SIDE_Y, SIDE_NY, SIDE_Z, SIDE_NZ, NB_SIDE=6 };
    eastl::array<tim::UVMesh, NB_SIDE> _planetSide;

    struct Batch
    {
        tim::Sphere sphere[NB_SIDE];
        eastl::array<BatchInstance, 4> lods;
    };

    tim::uint _gridResolution;
    eastl::vector<tim::uint> _gridIndex;

    eastl::array<eastl::array<Batch, 8>, 8> _grid;

    tim::uint indexGrid(tim::uint,tim::uint) const;
    tim::uint& indexGrid(tim::uint,tim::uint);

    void generateGrid(tim::uint);
    void generateBatchIndex(tim::uint, bool);
};

inline tim::uint Planet::indexGrid(tim::uint i,tim::uint j) const
{ return _gridIndex[j +  i*_gridResolution]; }

inline tim::uint& Planet::indexGrid(tim::uint i,tim::uint j)
{ return _gridIndex[j +  i*_gridResolution]; }

template<class Noise> void Planet::applyNoise(const Noise& noise, float factor)
{
    for(auto& side : _planetSide)
    {
        side.mapVertices([&](tim::vec3 v){
           v.normalize();
           v *= (1 + noise(v*0.5f+0.5f)*factor);
           return v;
        });
    }
}
