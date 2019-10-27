#pragma once

#include <EASTL/vector.h>
#include <EASTL/array.h>
#include <EASTL/string.h>

#include <fstream>

#include "math/Vector.h"
#include "math/Quaternion.h"
#include "core/ImageAlgorithm.h"
#include "math/Sphere.h"

namespace tim
{
    class Curve;
    class BaseMesh
	{
        friend class Curve;

    public:
        struct Face
        {
            eastl::array<uint,4> indexes; // support quads triangles, lines and points
            int nbIndexes;
        };

	public:
        BaseMesh() = default;
        ~BaseMesh() = default;
		
        BaseMesh(const BaseMesh&) = default;
        BaseMesh(BaseMesh&&) = default;
		
        BaseMesh& operator=(const BaseMesh&) = default;
        BaseMesh& operator=(BaseMesh&&) = default;

        BaseMesh scaled(vec3) const; // scale by a vec3 each vertice
        BaseMesh translated(vec3) const; // translate by a vec3 each vertice
        BaseMesh transformed(const mat4&) const;
        BaseMesh rotated(Quat) const;
        BaseMesh rotated(const mat3&) const;

        template<class T> BaseMesh& mapVertices(const T&);
        template<class T> BaseMesh& mapNormals(const T&);

        uint nbVertices() const;
		uint nbFaces() const;
        vec3 position(uint) const;

		const vec3* vertexData() const;
		eastl::vector<uint> indexData(uint nbPointsInFace = 3) const;

		size_t requestBufferSize(bool withNormal = true, bool withUv = false) const;
		void fillBuffer(void*, bool withNormal = true, bool withUv = false) const;

        void clearFaces();
        BaseMesh& addFace(const Face&);

        BaseMesh& operator+=(const BaseMesh&);

        void exportToObj(eastl::string) const;

        BaseMesh& computeNormals(bool correctSeems = true, int smooth = 0);
        BaseMesh& invertNormals();
        BaseMesh& invertFaces();

		static void computeJoinNormals(eastl::vector<BaseMesh*>&, int smooth = 0);

		vec3 vertex(uint) const;
		vec3 normal(uint) const;

		Sphere computeBoundingSphere();
		
    protected:
		eastl::vector<vec3> _vertices;
        eastl::vector<Face> _faces;
        eastl::vector<vec3> _normals;
        eastl::vector<vec2> _texCoords;

        eastl::vector<eastl::vector<uint>> _vertexToFaces; // for each vertex, we know the faces the vertex is in

        void buildVertexFaceMap(bool useRealPosition);

   private:
        std::ofstream& writeVertex(std::ofstream&, uint) const;

        vec3 faceNormal(uint) const;

	protected:
		static void generateGrid(BaseMesh&, vec2 size, uivec2 resolution, const ImageAlgorithm<float>&, float Zscale, bool withUV, bool triangulate);
	};

    inline BaseMesh& BaseMesh::addFace(const Face& face) { _faces.push_back(face); return *this; }
    inline uint BaseMesh::nbVertices() const { return _vertices.size(); }
	inline uint BaseMesh::nbFaces() const { return _faces.size(); }
	inline const vec3* BaseMesh::vertexData() const { return _vertices.data(); }

	inline vec3 BaseMesh::vertex(uint index) const { return _vertices[index]; }
	inline vec3 BaseMesh::normal(uint index) const { return _normals[index]; }

    template<class T> BaseMesh& BaseMesh::mapVertices(const T& tr)
    {
        for(auto& v : _vertices)
            v = tr(v);
        return *this;
    }

    template<class T> BaseMesh& BaseMesh::mapNormals(const T& tr)
    {
        for(auto& n : _normals)
            n = tr(n);
        return *this;
    }

	/* Mesh */

    class Mesh : public BaseMesh
    {
    public:
		using Vertex = vec3;

        Mesh() = default;
        ~Mesh() = default;
        Mesh(const Mesh&) = default;
        Mesh(Mesh&&) = default;

        Mesh(const BaseMesh& mesh);

        Mesh& operator=(const Mesh&) = default;
        Mesh& operator=(Mesh&&) = default;

        Mesh& addVertex(vec3);
		Mesh& addVertexAndNormal(vec3, vec3);

        Mesh& constructLine(vec3, vec3);
        Mesh& constructTriangle(vec3, vec3, vec3);
        Mesh& constructQuad(vec3, vec3, vec3, vec3);

		static Mesh generateGrid(vec2 size, uivec2 resolution, const ImageAlgorithm<float>&, float Zscale, bool triangulate = false);
    };

	inline Mesh& Mesh::addVertex(vec3 v) { _vertices.push_back(v); return *this; }
	inline Mesh& Mesh::addVertexAndNormal(vec3 v, vec3 n) { _vertices.push_back(v); _normals.push_back(n); return *this; }


	/* UV Mesh */

    class UVMesh : public BaseMesh
    {
    public:
        struct Vertex{ vec3 v; vec2 uv; };

        UVMesh() = default;
        ~UVMesh() = default;
        UVMesh(const UVMesh&) = default;
        UVMesh(UVMesh&&) = default;

        UVMesh(const BaseMesh& mesh);

        UVMesh& operator=(const UVMesh&) = default;
        UVMesh& operator=(UVMesh&&) = default;

        UVMesh& addVertex(const Vertex&);

        UVMesh& constructLine(const Vertex&, const Vertex&);
        UVMesh& constructTriangle(const Vertex&, const Vertex&, const Vertex&);
        UVMesh& constructQuad(const Vertex&, const Vertex&, const Vertex&, const Vertex&);

        UVMesh uv_scaled(vec2) const;

		static UVMesh generateGrid(vec2 size, uivec2 resolution, const ImageAlgorithm<float>&, float Zscale, bool triangulate = false);
    };

	inline UVMesh& UVMesh::addVertex(const Vertex& v) { _vertices.push_back(v.v); _texCoords.push_back(v.uv); return *this; }
}
