

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

    void Curve::tesselateCylindre(BaseMesh& mesh, const eastl::vector<uint>& bottom, const eastl::vector<uint>& top, uint resolution, bool triangulate)
    {
        for(uint i=0 ; i<resolution ; ++i)
        {
            if(triangulate)
            {
                mesh.addFace({{bottom[i%resolution], top[i%resolution], bottom[(i+1)%resolution], 0}, 3});
                mesh.addFace({{top[i%resolution], top[(i+1)%resolution], bottom[(i+1)%resolution], 0}, 3});
            }
            else
            {
                mesh.addFace({{bottom[i%resolution], top[i%resolution],
                           top[(i+1)%resolution], bottom[(i+1)%resolution]}, 4});
            }
        }
    }

    void Curve::tesselateCone(BaseMesh& mesh, const eastl::vector<uint>& bottom, uint top, uint resolution)
    {
        for(uint i=0 ; i<resolution ; ++i)
            mesh.addFace({{bottom[i%resolution], top, bottom[(i+1)%resolution], 0}, 3});
    }

    mat3 Curve::changeBasis(vec3 dir)
    {
        vec3 orthoDir = !fcompare(dir.dot(vec3(0,0,1)), 1, 0.001) ? dir.cross(vec3(0,0,1)) :
                       (!fcompare(dir.dot(vec3(0,1,0)), 1, 0.001) ? dir.cross(vec3(0,1,0)) : dir.cross(vec3(1,0,0)));

        mat3 base;
        base[0] = dir.cross(orthoDir);
        base[1] = orthoDir;
        base[2] = dir;
        return base.inverted();
    }
}
