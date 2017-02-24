#include "Mesh.h"
#include <iostream>

using namespace eastl;

namespace tim
{

/* BasicMesh */

void BaseMesh::exportToObj(string filename) const
{
    std::ofstream out(filename.c_str());
    if(!out)
    {
        std::cerr << "Unuable to open " << filename.c_str() << std::endl;
        return;
    }

    if(_vertices.empty())
        return;

    for(auto v : _vertices)
    {
        out << "v " << v.x() << " " << v.y() << " " << v.z() <<  "\n";
    }

    for(auto v : _normals)
    {
        out << "n " << v.x() << " " << v.y() << " " << v.z() <<  "\n";
    }

    for(auto v : _texCoords)
    {
        out << "t " << v.x() << " " << v.y() <<  "\n";
    }

    for(auto f : _faces)
    {
        switch(f.nbIndexes)
        {
        case 0:
        case 1:
        default:
            break;

        case 2:
            out << "l ";
            writeVertex(out, f.indexes[0]) << " ";
            writeVertex(out, f.indexes[1]) <<  "\n";
            break;

        case 3:
        case 4:
            out << "f ";
            for(int i=0 ; i<f.nbIndexes ; ++i)
                writeVertex(out, f.indexes[i]) << " ";
            out << "\n";
            break;
        }
    }
}

std::ofstream& BaseMesh::writeVertex(std::ofstream& out, uint index) const
{
    ++index;
    //if(_normals.empty() && _texCoords.empty())
    out << index;

    return out;
}


/* Mesh */

Mesh& Mesh::constructLine(vec3 p1, vec3 p2)
{
    _vertices.push_back(p1);
    _vertices.push_back(p2);
    _faces.push_back({{_vertices.size()-2, _vertices.size()-1, 0, 0}, 2});
    return *this;
}

Mesh& Mesh::constructTriangle(vec3 p1, vec3 p2, vec3 p3)
{
    _vertices.push_back(p1);
    _vertices.push_back(p2);
    _vertices.push_back(p3);
    _faces.push_back({{_vertices.size()-3, _vertices.size()-2, _vertices.size()-1, 0}, 3});
    return *this;
}

Mesh& Mesh::constructQuad(vec3 p1, vec3 p2, vec3 p3, vec3 p4)
{
    _vertices.push_back(p1);
    _vertices.push_back(p2);
    _vertices.push_back(p3);
    _vertices.push_back(p4);
    _faces.push_back({{_vertices.size()-4, _vertices.size()-3, _vertices.size()-2, _vertices.size()-1}, 4});
    return *this;
}

}
