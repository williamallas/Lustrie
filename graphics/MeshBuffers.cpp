#include "MeshBuffers.h"

MeshBuffers::MeshBuffers(const eastl::shared_ptr<dx12::GpuBuffer>& vb, const eastl::shared_ptr<dx12::GpuBuffer>& ib , size_t offset, int64_t numIndices)
	: _vb(vb), _ib(ib), _offset(offset), _numIndexes(numIndices)
{

}

MeshBuffers MeshBuffers::createFromMesh(const tim::BaseMesh& mesh, uint64_t* fence, tim::uint nbPointInFace, bool useNormal, bool useUV)
{
	if (mesh.nbVertices() <= 0)
		return MeshBuffers();

	size_t bufferSize = mesh.requestBufferSize(useNormal, useUV);
	auto indexBuffer = mesh.indexData(nbPointInFace);

	if (indexBuffer.empty() || bufferSize == 0)
		return MeshBuffers();

	dx12::GpuBuffer* vb = new dx12::GpuBuffer(mesh.nbVertices(), bufferSize / mesh.nbVertices());
	dx12::GpuBuffer* ib = new dx12::GpuBuffer(indexBuffer.size(), sizeof(tim::uint));
	
	byte* buffer_data = new byte[bufferSize];
	mesh.fillBuffer(buffer_data, true, true);

	auto& commandContext = dx12::CommandContext::AllocContext(dx12::CommandQueue::COPY);

	for (size_t i = 0; i < bufferSize; i += (1 << 20))
	{
		size_t numBytes = eastl::min(size_t(1 << 20), bufferSize - i);
		commandContext.initBuffer(*vb, buffer_data + i, numBytes, i);

		if (i > 0 && i % (20 << 20) == 0 && fence == nullptr)
			commandContext.flush(true);
	}

	delete buffer_data;
	bufferSize = indexBuffer.size() * sizeof(tim::uint);
	buffer_data = (byte*)indexBuffer.data();

	for (size_t i = 0; i < bufferSize; i += (1 << 20))
	{
		size_t numBytes = eastl::min(size_t(1 << 20), bufferSize - i);
		commandContext.initBuffer(*ib, buffer_data + i, numBytes, i);

		if (i > 0 && i % (20 << 20) == 0 && fence == nullptr)
			commandContext.flush(true);
	}

	if (fence != nullptr)
		*fence = commandContext.finish(false);
	else
		commandContext.finish(true);
		
	MeshBuffers res;
	res._vb = eastl::shared_ptr<dx12::GpuBuffer>(vb);
	res._ib = eastl::shared_ptr<dx12::GpuBuffer>(ib);
	return res;
}

eastl::shared_ptr<dx12::GpuBuffer> MeshBuffers::createVertexBufferFromMesh(const tim::BaseMesh& mesh, uint64_t* fence)
{
	if (mesh.nbVertices() <= 0)
		return eastl::shared_ptr<dx12::GpuBuffer>();

	size_t bufferSize = mesh.requestBufferSize(true, true);

	if (bufferSize == 0)
		return eastl::shared_ptr<dx12::GpuBuffer>();

	dx12::GpuBuffer* vb = new dx12::GpuBuffer(mesh.nbVertices(), bufferSize / mesh.nbVertices());

	byte* buffer_data = new byte[bufferSize];
	mesh.fillBuffer(buffer_data, true, true);

	auto& commandContext = dx12::CommandContext::AllocContext(dx12::CommandQueue::COPY);

	commandContext.initBuffer(*vb, buffer_data, bufferSize);

	if (fence != nullptr)
		*fence = commandContext.finish(false);
	else
		commandContext.finish(true);

	return eastl::shared_ptr<dx12::GpuBuffer>(vb);
}