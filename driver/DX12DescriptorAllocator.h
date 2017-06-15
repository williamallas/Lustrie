#pragma once

#include <core/NonCopyable.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/vector.h>
#include "DX12.h"
#include <mutex>

namespace dx12
{
	
	class Descriptor
	{

	public:
		Descriptor()
		{
			_cpuHandle.ptr = D3D12_CPU_VIRTUAL_ADDRESS_UNKNOWN;
			_gpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		}

		Descriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) : _cpuHandle(cpuHandle)
		{
			_gpuHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		}

		Descriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
			: _cpuHandle(cpuHandle), _gpuHandle(gpuHandle) {}

		Descriptor(const Descriptor&) = default;
		Descriptor& operator=(const Descriptor&) = default;

		Descriptor operator+(INT offsetScaledByDescriptorSize) const
		{
			Descriptor ret = *this;
			ret += offsetScaledByDescriptorSize;
			return ret;
		}

		void operator += (INT offsetScaledByDescriptorSize)
		{
			if (_cpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
				_cpuHandle.ptr += offsetScaledByDescriptorSize;
			if (_gpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
				_gpuHandle.ptr += offsetScaledByDescriptorSize;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle() const { return _cpuHandle; }
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle() const { return _gpuHandle; }

		bool isNull() const { return _cpuHandle.ptr == D3D12_CPU_VIRTUAL_ADDRESS_UNKNOWN; }
		bool isShaderVisible() const { return _gpuHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }

	private:
		D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle;
	};


	class DescriptorHeap : NonCopyable
	{
	public:
		DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE, uint32_t maxCount, bool shaderVisible);

		bool hasSpaceFor(uint32_t count) const { return count <= _numFreeDescriptors; }
		Descriptor alloc(uint32_t count = 1);

		Descriptor get(uint32_t index) const;
		bool validate(const Descriptor& descritpor) const;
		ID3D12DescriptorHeap* heapPtr() const;

	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _heap;

		uint32_t _descriptorSize;
		uint32_t _numFreeDescriptors;
		uint32_t _initialSize;

		Descriptor _firstHandle;
		Descriptor _nextFreeHandle;
	};

	inline ID3D12DescriptorHeap* DescriptorHeap::heapPtr() const { return _heap.Get(); }
	inline Descriptor DescriptorHeap::get(uint32_t offset) const { return _firstHandle + offset * _descriptorSize; }

	class DescriptorAllocator : NonCopyable
	{
	public:
		DescriptorAllocator();
		
		Descriptor alloc(uint32_t count);

	private:
		D3D12_DESCRIPTOR_HEAP_TYPE _type;
		std::mutex _mutex;
		eastl::vector<eastl::unique_ptr<DescriptorHeap>> _oldHeap;
		eastl::unique_ptr<DescriptorHeap> _currentHeap;

		static int _counter;
	};

	extern DescriptorAllocator g_descriptorsPool[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
}

