#include "DX12Resource.h"
#include <core/Logger.h>

namespace dx12
{
	GpuBuffer::GpuBuffer(uint32_t numElements, uint32_t elemSize) : Resource(), BufferBase(numElements, elemSize)
	{
		if (numElements*elemSize > 0)
			alloc(numElements, elemSize);
	}

	void GpuBuffer::alloc(uint32_t numElements, uint32_t elemSize)
	{
		_ASSERT(_resource.Get() == nullptr);

		D3D12_HEAP_PROPERTIES heapProps;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 0;
		heapProps.VisibleNodeMask = 0;
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

		_elemCount = numElements;
		_elemSize = elemSize;
		_bufferSize = elemSize * numElements;

		auto resDescription = genDescription();
		_currentState = D3D12_RESOURCE_STATE_COMMON;
		auto res = g_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDescription, _currentState, nullptr, IID_PPV_ARGS(_resource.GetAddressOf()));

		if (SUCCEEDED(res))
			_gpuVirtualAdress = _resource->GetGPUVirtualAddress();
		else
		{
			LOG("Unuable to alloc(gpuBuffer) ", numElements, " elements of size ", elemSize);
			_resource = nullptr;
		}
	}

	void GpuBuffer::clear()
	{
		_elemCount = 0;
		_elemSize = 0;
		_bufferSize = 0;

		_resource = nullptr;
		_gpuVirtualAdress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
	}

	D3D12_RESOURCE_DESC GpuBuffer::genDescription() const
	{
		D3D12_RESOURCE_DESC desc = {};
		desc.Alignment = 0;
		desc.DepthOrArraySize = 1;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Flags = _resourceFlags;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Height = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Width = (UINT64)_bufferSize;
		return desc;
	}


	/******************************/
	/*     CPU writable buffer    */
	/******************************/

	CpuWritableBuffer::CpuWritableBuffer(uint32_t numElements, uint32_t elemSize) : BufferBase(numElements, elemSize), _cpuVirtualAdress(nullptr)
	{
		if(numElements*elemSize > 0)
			alloc(numElements, elemSize);
	}

	void CpuWritableBuffer::alloc(uint32_t numElements, uint32_t elemSize)
	{
		_ASSERT(_resource.Get() == nullptr);

		D3D12_HEAP_PROPERTIES heapProps;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 0;
		heapProps.VisibleNodeMask = 0;
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC resourceDesc;
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Width = numElements * elemSize;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		_currentState = D3D12_RESOURCE_STATE_GENERIC_READ;
		auto res = g_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
												     _currentState, nullptr, IID_PPV_ARGS(_resource.GetAddressOf()));

		if (SUCCEEDED(res))
		{
			_gpuVirtualAdress = _resource->GetGPUVirtualAddress();
			_cpuVirtualAdress = nullptr;

			_bufferSize = numElements * elemSize;
			_elemCount = numElements;
			_elemSize = elemSize;
		}
		else
		{
			LOG("Unable to alloc(CpuWritableBuffer) ", numElements, " elements of size ", elemSize);
			_resource = nullptr;

			_bufferSize = 0;
			_elemCount = 0;
			_elemSize = 0;
		}
	}

	void CpuWritableBuffer::clear()
	{
		if (_resource.Get() != nullptr)
		{
			if (_cpuVirtualAdress != nullptr)
				unmap();

			_resource = nullptr;
			_gpuVirtualAdress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
		}
	}

	void* CpuWritableBuffer::map()
	{
		_ASSERT(_cpuVirtualAdress == nullptr);
		auto res = _resource->Map(0, nullptr, &_cpuVirtualAdress);
		_ASSERT(SUCCEEDED(res));

		return _cpuVirtualAdress;
	}

	void CpuWritableBuffer::unmap()
	{
		_ASSERT(_cpuVirtualAdress != nullptr);
		_resource->Unmap(0, nullptr);
		_cpuVirtualAdress = nullptr;
	}

	/********************/
	/*     Resource     */
	/********************/

	D3D12_VERTEX_BUFFER_VIEW BaseResource::vertexBufferView(uint32_t numVertices, uint32_t stride, uint32_t offset) const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = _gpuVirtualAdress + offset;
		vbv.SizeInBytes = numVertices * stride;
		vbv.StrideInBytes = stride;
		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW BaseResource::indexBufferView(uint32_t numIndices, uint32_t offset, bool is32bit) const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = _gpuVirtualAdress + offset;
		ibv.Format = is32bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		ibv.SizeInBytes = numIndices * (is32bit ? 4 : 2);
		return ibv;
	}
}