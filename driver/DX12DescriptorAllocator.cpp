#include "DX12DescriptorAllocator.h"
#include <core/Logger.h>

namespace dx12
{
	DescriptorHeap::DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t maxCount, bool shaderVisible)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
		heapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0;
		heapDesc.NumDescriptors = maxCount;
		heapDesc.Type = type;

		auto result = g_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(_heap.GetAddressOf()));
		_ASSERT(SUCCEEDED(result));

		_descriptorSize = g_device->GetDescriptorHandleIncrementSize(type);
		_initialSize = maxCount;
		_numFreeDescriptors = maxCount;
		_firstHandle = Descriptor(_heap->GetCPUDescriptorHandleForHeapStart(), _heap->GetGPUDescriptorHandleForHeapStart());
		_nextFreeHandle = _firstHandle;
	}

	Descriptor DescriptorHeap::alloc(uint32_t count)
	{
		_ASSERT(hasSpaceFor(count));
		Descriptor ret = _nextFreeHandle;
		_nextFreeHandle += count * _descriptorSize;
		return ret;
	}

	bool DescriptorHeap::validate(const Descriptor& descritpor) const
	{
		if (descritpor.cpuHandle().ptr < _firstHandle.cpuHandle().ptr ||
			descritpor.cpuHandle().ptr >= _firstHandle.cpuHandle().ptr + _initialSize * _descriptorSize)
			return false;

		if (descritpor.gpuHandle().ptr - _firstHandle.gpuHandle().ptr !=
			descritpor.cpuHandle().ptr - _firstHandle.cpuHandle().ptr)
			return false;

		return true;
	}

	Descriptor DescriptorAllocator::alloc(uint32_t count)
	{
		std::lock_guard<std::mutex> guard(_mutex);

		if (_currentHeap == nullptr)
			_currentHeap = eastl::make_unique<DescriptorHeap>(_type, 128, false);

		if (!_currentHeap->hasSpaceFor(count))
		{
			_oldHeap.emplace_back(eastl::move(_currentHeap));
			_currentHeap = eastl::make_unique<DescriptorHeap>(_type, 128, false);
		}

		_ASSERT(_currentHeap->hasSpaceFor(count));

		return _currentHeap->alloc(count);
	}

	int DescriptorAllocator::_counter = 0;

	DescriptorAllocator::DescriptorAllocator() : _type((D3D12_DESCRIPTOR_HEAP_TYPE)_counter++) 
	{
		_ASSERT(_counter <= D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES);
	}

	DescriptorAllocator g_descriptorsPool[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
}