#include "Mesh.h"
#include <iostream>
#include <EASTL/unordered_map.h>

using namespace eastl;

namespace tim
{

/* BasicMesh */

vec3 BaseMesh::position(uint index) const
{
    if(index < _vertices.size())
        return _vertices[index];
    else
        return vec3();
}

BaseMesh& BaseMesh::operator+=(const BaseMesh& mesh)
{
    if(_vertices.empty())
        return (*this = mesh);

    uint startIndex = _vertices.size();
    for(size_t i=0 ; i<mesh._vertices.size() ; ++i)
    {
        _vertices.push_back(mesh._vertices[i]);

        if(!_normals.empty() && !mesh._normals.empty())
            _normals.push_back(mesh._normals[i]);

        if(!_texCoords.empty() && !mesh._texCoords.empty())
            _texCoords.push_back(mesh._texCoords[i]);
    }

    for(size_t i=0 ; i<mesh._faces.size() ; ++i)
    {
        _faces.push_back({{mesh._faces[i].indexes[0] + startIndex,
                           mesh._faces[i].indexes[1] + (mesh._faces[i].nbIndexes>1?startIndex:0),
                           mesh._faces[i].indexes[2] + (mesh._faces[i].nbIndexes>2?startIndex:0),
                           mesh._faces[i].indexes[3] + (mesh._faces[i].nbIndexes>3?startIndex:0)}, mesh._faces[i].nbIndexes});
    }

    return *this;
}

BaseMesh BaseMesh::scaled(vec3 scale) const
{
    return BaseMesh(*this).mapVertices([&](vec3 v){ return v*scale; })
                          .mapNormals([&](vec3 n){ return (n*scale).normalized(); });
}

BaseMesh BaseMesh::translated(vec3 translation) const
{
    return BaseMesh(*this).mapVertices([&](vec3 v){ return v+translation; });
}


BaseMesh BaseMesh::transformed(const mat4& tr) const
{
    mat3 rot = tr.to<3>();
    return BaseMesh(*this).mapVertices([&](vec3 v){ return tr*v; })
                          .mapNormals([&](vec3 n) { return rot*n; });
}

BaseMesh BaseMesh::rotated(Quat rot) const
{
    return BaseMesh(*this).mapVertices(rot).mapNormals(rot);
}

BaseMesh BaseMesh::rotated(const mat3& rot) const
{
    return BaseMesh(*this).mapVertices([&](vec3 v){ return rot*v; })
                          .mapNormals([&](vec3 n) { return rot*n; });
}

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

BaseMesh& BaseMesh::computeNormals(bool correctSeems, int smooth)
{
    _normals.clear();
    _normals.resize(_vertices.size());

    buildVertexFaceMap(correctSeems);

    for(size_t i=0 ; i<_vertices.size() ; ++i)
    {
        vec3 n;
        for(uint faceIndex : _vertexToFaces[i])
            n += faceNormal(faceIndex);

        if(n != vec3(0,0,0))
            _normals[i] = n.normalized();
    }

	if (smooth > 0)
	{
		eastl::vector<vec3> tmpNormals(_vertices.size());
		for (int s = 0; s < smooth; ++s)
		{
			for (size_t i = 0; i < _vertices.size(); ++i)
			{
				vec3 n = _normals[i]; uint nb = 1;
				for (uint faceIndex : _vertexToFaces[i])
				{
					if (_faces[faceIndex].nbIndexes >= 3)
					{
						if(i != _faces[faceIndex].indexes[0])
							n += _normals[_faces[faceIndex].indexes[0]];

						if (i != _faces[faceIndex].indexes[1])
							n += _normals[_faces[faceIndex].indexes[1]];

						if (i != _faces[faceIndex].indexes[2])
							n += _normals[_faces[faceIndex].indexes[2]];

						nb += 2;
					}
				}

				n /= float(nb);
				tmpNormals[i] = n.normalized();
			}

			eastl::swap(_normals, tmpNormals);
		}
	}

    return *this;
}

BaseMesh& BaseMesh::invertNormals()
{
    return mapNormals([](vec3 n) { return -n; });
}

BaseMesh& BaseMesh::invertFaces()
{
    for(auto& face : _faces)
    {
        if(face.nbIndexes > 2)
            eastl::swap(face.indexes[0], face.indexes[1]);

        if(face.nbIndexes == 4)
            eastl::swap(face.indexes[2], face.indexes[3]);
    }

    return *this;
}

size_t BaseMesh::requestBufferSize(bool withNormal, bool withUv) const
{
	size_t res = nbVertices() * sizeof(vec3);
	if(withNormal && !_normals.empty())
		res += nbVertices() * sizeof(vec3);
	if (withUv && !_normals.empty())
		res += nbVertices() * sizeof(vec2);

	return res;
}

void BaseMesh::fillBuffer(void* ptr, bool withNormal, bool withUv) const
{
	size_t stride = sizeof(vec3) + ((withNormal && !_normals.empty()) ? sizeof(vec3) : 0) + ((withUv && !_texCoords.empty()) ? sizeof(vec2) : 0);
	byte* bptr = (byte*)ptr;

	for (size_t i = 0; i < nbVertices(); ++i)
	{
		size_t offset = 0;
		memcpy(bptr + offset, &_vertices[i], sizeof(vec3));
		offset += sizeof(vec3);

		if (withNormal && !_normals.empty())
		{
			memcpy(bptr + offset, &_normals[i], sizeof(vec3));
			offset += sizeof(vec3);
		}

		if (withUv && !_texCoords.empty())
		{
			memcpy(bptr + offset, &_texCoords[i], sizeof(vec2));
			offset += sizeof(vec2);
		}

		bptr += stride;
	}
}

namespace
{
    struct HashVec3
    {
        size_t operator()(const vec3& v) const
        { return v.hash<3>(); }
    };
}

void BaseMesh::buildVertexFaceMap(bool useRealPosition)
{
    _vertexToFaces.clear();
    _vertexToFaces.resize(_vertices.size(), eastl::vector<uint>());

    if(!useRealPosition)
    {
        for(size_t indexFace=0 ; indexFace < _faces.size() ; ++indexFace)
        {
            for(int i=0 ; i<_faces[indexFace].nbIndexes ; ++i)
                _vertexToFaces[_faces[indexFace].indexes[i]].push_back(indexFace);
        }
    }
    else
    {
        eastl::unordered_map<vec3, vector<uint>, HashVec3> positionToVertex;
        for(size_t i=0 ; i<_vertices.size() ; ++i)
            positionToVertex[_vertices[i]].push_back(i);

        for(size_t indexFace=0 ; indexFace < _faces.size() ; ++indexFace)
        {
            for(int i=0 ; i<_faces[indexFace].nbIndexes ; ++i)
            {
                auto cousin = positionToVertex[_vertices[_faces[indexFace].indexes[i]]];

                for(uint v_id : cousin)
                    _vertexToFaces[v_id].push_back(indexFace);
            }
        }
    }
}

vec3 BaseMesh::faceNormal(uint faceIndex) const
{
    const Face& face = _faces[faceIndex];
    if(face.nbIndexes <= 2)
        return vec3();

    vec3 v1 = _vertices[face.indexes[2]] - _vertices[face.indexes[1]];
    vec3 v2 = _vertices[face.indexes[0]] - _vertices[face.indexes[1]];

    return v1.cross(v2).normalized();
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
    uint curIndex = 0;
	
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

void BaseMesh::clearFaces()
{
    _faces.clear();
}

eastl::vector<uint> BaseMesh::indexData() const
{
	eastl::vector<uint> data;
	data.reserve(_faces.size() * 3);

	for (const auto& f : _faces)
	{
		if (f.nbIndexes >= 3)
			data.insert(data.end(), f.indexes.begin(), f.indexes.begin() + 3);
	}

	return data;
}

void BaseMesh::computeJoinNormals(eastl::vector<BaseMesh*>& meshs, int smooth)
{
	BaseMesh joined;
	for (auto m : meshs)
		joined += *m;

	joined.computeNormals(true, smooth);

	size_t counter = 0;
	for (size_t i = 0; i < meshs.size(); ++i)
	{
		meshs[i]->_normals.resize(meshs[i]->nbVertices());
		for (size_t j = 0; j < meshs[i]->nbVertices(); ++j)
			meshs[i]->_normals[j] = joined._normals[j + counter];

		counter += meshs[i]->nbVertices();
	}
}

/* Mesh */

Mesh::Mesh(const BaseMesh& mesh) : BaseMesh(mesh) {}

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

UVMesh::UVMesh(const BaseMesh& mesh) : BaseMesh(mesh) {}

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

UVMesh UVMesh::uv_scaled(vec2 scale) const
{
    UVMesh res = *this;
    for(vec2& t : res._texCoords)
        t *= scale;
    return res;
}

}
