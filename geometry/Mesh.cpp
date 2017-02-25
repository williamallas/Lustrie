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
        out << "vn " << v.x() << " " << v.y() << " " << v.z() <<  "\n";
    }

    for(auto v : _texCoords)
    {
        out << "vt " << v.x() << " " << v.y() <<  "\n";
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
    if(_normals.empty() && _texCoords.empty())
		out << index;
	else if(_normals.empty())
		out << index << "/" << index;
	else if (_texCoords.empty())
		out << index << "//" << index;
	else
		out << index << "/" << index << "/" << index;

    return out;
}

void BaseMesh::generateGrid(BaseMesh& mesh, vec2 size, uivec2 resolution, const ImageAlgorithm<float>& heightmap, float Zscale, bool withUV, bool triangulate)
{
	if (resolution.x() <= 1 || resolution.y() <= 1)
		return;

	vec2 d = { size.x() / (resolution.x()-1), size.y() / (resolution.y()-1) };
	vec2 d_img;

	if(!heightmap.empty())
		d_img = vec2(float(heightmap.size().x()) / (resolution.x() - 1), float(heightmap.size().y()) / (resolution.y() - 1));

	eastl::vector<eastl::vector<uint>> indexes(resolution.x(), eastl::vector<uint>(resolution.y()));
	uint curIndex = 0, initial = mesh._vertices.size();
	
	for (uint i = 0; i < resolution.x(); ++i)
	{
		for (uint j = 0; j < resolution.y(); ++j)
		{
			vec3 p = vec3(d.x()*i, d.y()*j, 0) - vec3(size*0.5, 0);

			if (!heightmap.empty())
				p.z() = heightmap.getSmooth(d_img * vec2(i, j)) * Zscale;

			mesh._vertices.push_back(p);
			indexes[i][j] = curIndex++;

			if (withUV)
			{
				vec2 uv(float(i) / (resolution.x() - 1), float(j) / (resolution.y() - 1));
				mesh._texCoords.push_back(uv);
			}

			if (i > 0 && j > 0)
			{
				if (triangulate)
				{
					mesh.addFace({ { indexes[i - 1][j - 1], indexes[i][j - 1], indexes[i][j], 0 }, 3 });
					mesh.addFace({ { indexes[i][j], indexes[i-1][j], indexes[i-1][j-1], 0 }, 3 });
				}
				else
					mesh.addFace({ { indexes[i - 1][j - 1], indexes[i][j - 1], indexes[i][j], indexes[i - 1][j] }, 4 });
			}
		}
	}
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

Mesh Mesh::generateGrid(vec2 size, uivec2 resolution, const ImageAlgorithm<float>& heightmap, float Zscale, bool triangulate)
{
	Mesh m;
	BaseMesh::generateGrid(m, size, resolution, heightmap, Zscale, false, triangulate);
	return m;
}

/* UVMesh */

UVMesh& UVMesh::constructLine(const Vertex& p1, const Vertex& p2)
{
    _vertices.push_back(p1.v);
    _vertices.push_back(p2.v);

    _texCoords.push_back(p1.uv);
    _texCoords.push_back(p2.uv);

    _faces.push_back({{_vertices.size()-2, _vertices.size()-1, 0, 0}, 2});
    return *this;
}

UVMesh& UVMesh::constructTriangle(const Vertex& p1, const Vertex& p2, const Vertex& p3)
{
    _vertices.push_back(p1.v);
    _vertices.push_back(p2.v);
    _vertices.push_back(p3.v);

    _texCoords.push_back(p1.uv);
    _texCoords.push_back(p2.uv);
    _texCoords.push_back(p3.uv);

    _faces.push_back({{_vertices.size()-3, _vertices.size()-2, _vertices.size()-1, 0}, 3});
    return *this;
}

UVMesh& UVMesh::constructQuad(const Vertex& p1, const Vertex& p2, const Vertex& p3, const Vertex& p4)
{
    _vertices.push_back(p1.v);
    _vertices.push_back(p2.v);
    _vertices.push_back(p3.v);
    _vertices.push_back(p4.v);

    _texCoords.push_back(p1.uv);
    _texCoords.push_back(p2.uv);
    _texCoords.push_back(p3.uv);
    _texCoords.push_back(p4.uv);

    _faces.push_back({{_vertices.size()-4, _vertices.size()-3, _vertices.size()-2, _vertices.size()-1}, 4});
    return *this;
}

UVMesh UVMesh::generateGrid(vec2 size, uivec2 resolution, const ImageAlgorithm<float>& heightmap, float Zscale, bool triangulate)
{
	UVMesh m;
	BaseMesh::generateGrid(m, size, resolution, heightmap, Zscale, true, triangulate);
	return m;
}

}
