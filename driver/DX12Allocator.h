
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.

#pragma once

#include "DX12Resource.h"
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/queue.h>
#include <mutex>
#include <core/type.h>

namespace dx12
{
	// Constant blocks must be multiples of 16 constants @ 16 bytes each
	static const int DEFAULT_ALIGN = 256;

	// Various types of allocations may contain NULL pointers. 
	struct DynAlloc
	{
		DynAlloc(BaseResource& baseResource, size_t offset, size_t size)
			: buffer(baseResource), inBufferOffset(offset), sizeAlloc(size) {}

		DynAlloc(DynAlloc&&) = default;
		DynAlloc& operator=(DynAlloc&&) = default;

		BaseResource& buffer;	// The buffer the allocation lies in.
		size_t inBufferOffset;			
		size_t sizeAlloc;			
		void* dataPtr;			// The CPU-writeable address
		D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;	// The GPU-visible address
	};

	class LinearAllocationPage : public BaseResource
	{
	public:
		LinearAllocationPage(ID3D12Resource* resource, D3D12_RESOURCE_STATES state) : BaseResource(resource, resource->GetGPUVirtualAddress())
		{
			_currentState = state;
			_cpuVirtualAddress = nullptr;
			resource->Map(0, nullptr, &_cpuVirtualAddress);
		}

		~LinearAllocationPage()
		{ unmap(); }

		void map()
		{
			if (_cpuVirtualAddress == nullptr)
				_resource->Map(0, nullptr, &_cpuVirtualAddress);
		}

		void unmap(void)
		{
			if (_cpuVirtualAddress != nullptr)
			{
				_resource->Unmap(0, nullptr);
				_cpuVirtualAddress = nullptr;
			}
		}

		void* _cpuVirtualAddress;
	};

	enum LinearAllocatorType
	{
		GpuExclusive = 0,		// DEFAULT   GPU-writeable (via UAV)
		CpuWritable = 1,		// UPLOAD CPU-writeable (but write combined)

		NumAllocatorTypes
	};

	class LinearAllocatorPageManager
	{
	public:
		enum
		{
			GpuAllocatorPageSize = 1 << 16,    // 64K
			CpuAllocatorPageSize = 2 << 20     // 2MB
		};

		LinearAllocatorPageManager();
		LinearAllocationPage* requestPage();
		LinearAllocationPage* createNewPage(size_t pageSize = 0);

		// Discarded pages will get recycled.  This is for fixed size pages.
		void discardPages(uint64_t fence, const eastl::vector<LinearAllocationPage*>& pages);

		// Freed pages will be destroyed once their fence has passed.  This is for single-use, "large" pages.
		void freeLargePages(uint64_t fence, const eastl::vector<LinearAllocationPage*>& pages);

		void destroy() { _pagePool.clear(); }

	private:
		static LinearAllocatorType _allocatorTypeEnumerator;

		LinearAllocatorType _allocatorType;
		eastl::vector<std::unique_ptr<LinearAllocationPage> > _pagePool;
		eastl::queue<std::pair<uint64_t, LinearAllocationPage*> > _retiredPages;
		eastl::queue<std::pair<uint64_t, LinearAllocationPage*> > _deletionQueue;
		eastl::queue<LinearAllocationPage*> _availablePages;
		std::mutex _mutex;
	};

	class LinearAllocator
	{
	public:

		LinearAllocator(LinearAllocatorType type) : _allocatorType(type), _pageSize(0), _curOffset(~(size_t)0), _curPage(nullptr)
		{
			_pageSize = (type == GpuExclusive ? LinearAllocatorPageManager::GpuAllocatorPageSize :
				                                LinearAllocatorPageManager::CpuAllocatorPageSize);
		}

		DynAlloc allocate(size_t SizeInBytes, size_t Alignment = DEFAULT_ALIGN);

		void cleanup(uint64_t FenceID);

		static void destroyAll()
		{
			_pageManager[0].destroy();
			_pageManager[1].destroy();
		}

	private:
		DynAlloc allocateLargePage(size_t sizeInBytes);

		static LinearAllocatorPageManager _pageManager[2];

		LinearAllocatorType _allocatorType;
		size_t _pageSize;
		size_t _curOffset;
		LinearAllocationPage* _curPage;

		eastl::vector<LinearAllocationPage*> _retiredPages;
		eastl::vector<LinearAllocationPage*> _largePageList;
	};
	
}