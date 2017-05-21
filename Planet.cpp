#include "Planet.h"

using namespace tim;

Planet::Planet(uint resolution)
{
    generateGrid(resolution);
}

void Planet::generateGrid(uint res)
{
    _gridIndex.resize((res+1)*(res+1));
    _gridResolution = res+1;
    UVMesh mesh = UVMesh();

    float d = { 1.f / res };

    uint curIndex = 0;
    //bool triangulate = false;
    for (uint i = 0; i < res+1; ++i)
    {
        for (uint j = 0; j < res+1; ++j)
        {
            vec3 p = vec3(d*i, d*j, 0) - vec3(0.5, 0.5, 0);
            vec2 uv(float(i) / (res-1), float(j) / (res-1));
            mesh.addVertex({p,uv});

            indexGrid(i,j) = curIndex++;

            /*if (i > 0 && j > 0)
            {
                if (triangulate)
                {
                    mesh.addFace({ { indexGrid(i-1,j-1), indexGrid(i,j-1), indexGrid(i,j), 0 }, 3 });
                    mesh.addFace({ { indexGrid(i,j), indexGrid(i-1,j), indexGrid(i-1,j-1), 0 }, 3 });
                }
                else
                    mesh.addFace({ { indexGrid(i-1,j-1), indexGrid(i,j-1), indexGrid(i,j), indexGrid(i-1,j) }, 4 });
            }*/
        }
    }

    _planetSide[SIDE_X] = mesh.rotated(mat3::RotationY(toRad(90))).translated(vec3(0.5,0,0));
    _planetSide[SIDE_NX] = mesh.rotated(mat3::RotationY(toRad(-90))).translated(vec3(-0.5,0,0));

    _planetSide[SIDE_Y] = mesh.rotated(mat3::RotationX(toRad(-90))).translated(vec3(0,0.5,0));
    _planetSide[SIDE_NY] = mesh.rotated(mat3::RotationX(toRad(90))).translated(vec3(0,-0.5,0));

    _planetSide[SIDE_Z] = mesh.translated(vec3(0,0,0.5));
    _planetSide[SIDE_NZ] = mesh.rotated(mat3::RotationX(toRad(180))).translated(vec3(0,0,-0.5));

    generateBatchIndex(res, false);


    //mesh = mesh.mapVertices([](vec3 v) { return (v+vec3(0,0,0.5)).normalized(); });
}

void Planet::generateBatchIndex(tim::uint res, bool triangulate)
{
    for(auto& l : _grid)
        for(auto& r : l)
            for(auto& lod : r.lods)
                lod.clear();

    for(uint i=0 ; i<_grid.size() ; ++i)
    {
        for(uint j=0 ; j<_grid.size() ; ++j)
        {
            for(uint lod = 0 ; lod < _grid[i][j].lods.size() ; ++lod)
            {
                uint batchRes = res / _grid.size();
                uint stride = 1 << lod;

                for (uint x = 0; x < batchRes+1; x += stride)
                {
                    for (uint y = 0; y < batchRes+1; y += stride)
                    {
                        uint ox = i*batchRes;
                        uint oy = j*batchRes;
                        uint xx=x+ox, yy=y+oy;

                        if (x > 0 && y > 0)
                        {
                            if (triangulate)
                            {
                                _grid[i][j].lods[lod].push_back({ { indexGrid(xx-stride,yy-stride), indexGrid(xx,yy-stride), indexGrid(xx,yy), 0 }, 3 });
                                _grid[i][j].lods[lod].push_back({ { indexGrid(xx,yy), indexGrid(xx-stride,yy), indexGrid(xx-stride,yy-stride), 0 }, 3 });
                            }
                            else
                                _grid[i][j].lods[lod].push_back({ { indexGrid(xx-stride,yy-stride), indexGrid(xx,yy-stride), indexGrid(xx,yy), indexGrid(xx-stride,yy) }, 4 });
                        }
                    }
                }
            }
            for(int counter=0 ; counter<NB_SIDE ; ++counter)
            {
                _grid[i][j].sphere[counter] = Sphere((_planetSide[counter].position(_grid[i][j].lods[0][0].indexes[0]) +
                                                      _planetSide[counter].position(_grid[i][j].lods[0].back().indexes[0])) * 0.5, 1);
            }
        }
    }


}

tim::UVMesh Planet::generateMesh(vec3 pos) const
{
    UVMesh result;
    for(int side=0 ; side<NB_SIDE ; ++side)
    {
        UVMesh sideMesh = _planetSide[side];
        for(uint i=0 ; i<_grid.size() ; ++i)
        {
            for(uint j=0 ; j<_grid.size() ; ++j)
            {
                //int index = int( (_grid[i][j].sphere.center() - vec3(0,0,1)).length() * 4 + 0.5);
                for(auto f : _grid[i][j].lods[0])
                    sideMesh.addFace(f);
            }
        }
        result += sideMesh;
    }
    result.computeNormals(false);

    return result;
}
