#pragma once

#include "DX12Resource.h"
namespace dx12
{
	class BufferBase
	{
	public:
		BufferBase() = default;
		BufferBase(uint32_t numElements, uint32_t elemSize) : _bufferSize(numElements*elemSize), _elemCount(numElements), _elemSize(elemSize) {}

		size_t bufferSize() const { return _bufferSize; }
		uint32_t elemCount() const { return _elemCount; }
		uint32_t elemSize() const { return _elemSize; }

	protected:
		size_t _bufferSize = 0;
		uint32_t _elemCount = 0;
		uint32_t _elemSize = 0;
	};

	class GpuBuffer : public Resource, public BufferBase
	{
	public:
		GpuBuffer() = default;
		GpuBuffer(uint32_t numElements, uint32_t elemSize);
		~GpuBuffer() { clear(); }

		void alloc(uint32_t numElements, uint32_t elemSize);
		void clear();

		using Resource::vertexBufferView;
		using Resource::indexBufferView;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView() const { return vertexBufferView(_elemCount, _elemSize, 0); }
		D3D12_INDEX_BUFFER_VIEW indexBufferView(bool is32bit = true) const { return indexBufferView(_elemCount, 0, _elemSize == 4); }

	private:
		D3D12_RESOURCE_FLAGS _resourceFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		D3D12_RESOURCE_DESC genDescription() const;
	};

	class CpuWritableBuffer : public BaseResource, public BufferBase
	{
	public:
		CpuWritableBuffer() = default;
		CpuWritableBuffer(uint32_t numElements, uint32_t elemSize);
		~CpuWritableBuffer() { clear(); }

		void alloc(uint32_t numElements, uint32_t elemSize);
		void clear();

		void* map();
		void unmap();

		using BaseResource::vertexBufferView;
		using BaseResource::indexBufferView;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView() const { return vertexBufferView(_elemCount, _elemSize, 0); }
		D3D12_INDEX_BUFFER_VIEW indexBufferView(bool is32bit = true) const { return indexBufferView(_elemCount, 0, _elemSize == 4); }

	protected:
		void* _cpuVirtualAdress = nullptr;
	};
}


