#pragma once

#include "API.h"
#include <EASTL/shared_ptr.h>
#include <geometry\Mesh.h>

class MeshBuffers
{
	friend class Graphics;

public:
	MeshBuffers() = default;
	MeshBuffers(const MeshBuffers&) = default;
	MeshBuffers& operator=(const MeshBuffers&) = default;
	MeshBuffers(const eastl::shared_ptr<dx12::GpuBuffer>& vb, const eastl::shared_ptr<dx12::GpuBuffer>& ib, size_t offset=0, int64_t numIndices=0);

	const eastl::shared_ptr<dx12::GpuBuffer>& vb() const { return _vb; }
	const eastl::shared_ptr<dx12::GpuBuffer>& ib() const { return _ib; }

	static MeshBuffers createFromMesh(const tim::BaseMesh&, uint64_t* fence = nullptr);
	static eastl::shared_ptr<dx12::GpuBuffer> createVertexBufferFromMesh(const tim::BaseMesh&, uint64_t* fence = nullptr);

	void setOffset(size_t);
	void setNumIndices(int64_t);

	size_t offset() const;
	int64_t numIndices() const;

private:
	eastl::shared_ptr<dx12::GpuBuffer> _vb, _ib;
	size_t _offset = 0;
	int64_t _numIndexes = -1;
};

inline void MeshBuffers::setOffset(size_t o) { _offset = o; }
inline void MeshBuffers::setNumIndices(int64_t n) { _numIndexes = n; }

inline size_t MeshBuffers::offset() const { return _offset; }
inline int64_t MeshBuffers::numIndices() const { return _numIndexes; }