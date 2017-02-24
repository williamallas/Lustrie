

#include "Curve.h"

namespace tim
{
    Mesh Curve::convertToMesh() const
    {
        Mesh mesh;
        if(_points.empty())
            return mesh;

        mesh._vertices = _points;
        for(size_t i=0 ; i<_points.size()-1 ; ++i)
            mesh.addFace(Mesh::Face({{i,i+1,0,0}, 2}));

        if(_closed)
            mesh.addFace(Mesh::Face({{_points.size()-1,0,0,0}, 2}));

        return mesh;
    }

    void Curve::tesselateCylindre(BaseMesh& mesh, const eastl::vector<uint>& bottom, const eastl::vector<uint>& top, uint resolution)
    {
        for(uint i=0 ; i<resolution ; ++i)
        {
            mesh.addFace({{bottom[i%resolution], top[i%resolution],
                           top[(i+1)%resolution], bottom[(i+1)%resolution]}, 4});
        }
    }

    void Curve::tesselateCone(BaseMesh& mesh, const eastl::vector<uint>& bottom, uint top, uint resolution)
    {
        for(uint i=0 ; i<resolution ; ++i)
            mesh.addFace({{bottom[i%resolution], top, bottom[(i+1)%resolution], 0}, 3});
    }
}
