#include "MeshBuffers.h"

MeshBuffers::MeshBuffers(const eastl::shared_ptr<dx12::GpuBuffer>& vb, const eastl::shared_ptr<dx12::GpuBuffer>& ib , size_t offset, int64_t numIndices)
	: _vb(vb), _ib(ib), _offset(offset), _numIndexes(numIndices)
{

}

MeshBuffers MeshBuffers::createFromMesh(const tim::BaseMesh& mesh, uint64_t* fence)
{
	if (mesh.nbVertices() <= 0)
		return MeshBuffers();

	size_t bufferSize = mesh.requestBufferSize(true, true);
	auto indexBuffer = mesh.indexData();

	if (indexBuffer.empty() || bufferSize == 0)
		return MeshBuffers();

	dx12::GpuBuffer* vb = new dx12::GpuBuffer(mesh.nbVertices(), bufferSize / mesh.nbVertices());
	dx12::GpuBuffer* ib = new dx12::GpuBuffer(indexBuffer.size(), sizeof(tim::uint));
	
	byte* buffer_data = new byte[bufferSize];
	mesh.fillBuffer(buffer_data, true, true);

	auto& commandContext = dx12::CommandContext::AllocContext(dx12::CommandQueue::COPY);

	commandContext.initBuffer(*vb, buffer_data, bufferSize);
	commandContext.initBuffer(*ib, indexBuffer.data(), indexBuffer.size() * sizeof(tim::uint));

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