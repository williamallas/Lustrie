#include "DX12Allocator.h"
#include "DX12.h"
#include "DX12CommandQueue.h"
#include <core/Logger.h>
#include <iostream>

namespace dx12
{
	LinearAllocatorPageManager LinearAllocator::_pageManager[2];

	LinearAllocatorType LinearAllocatorPageManager::_allocatorTypeEnumerator = LinearAllocatorType::GpuExclusive;


	/*********************/
	/* Allocator manager */
	/*********************/

	LinearAllocatorPageManager::LinearAllocatorPageManager()
	{
		_allocatorType = _allocatorTypeEnumerator;
		_allocatorTypeEnumerator = (LinearAllocatorType)(_allocatorTypeEnumerator + 1);
		_ASSERT(_allocatorTypeEnumerator <= NumAllocatorTypes);
	}

	

	LinearAllocationPage* LinearAllocatorPageManager::requestPage()
	{
		std::lock_guard<std::mutex> lockGuard(_mutex);

		while (!_retiredPages.empty() && g_commandQueues->isFenceComplete(_retiredPages.front().first))
		{
			_availablePages.push(_retiredPages.front().second);
			_retiredPages.pop();
		}

		LinearAllocationPage* pagePtr = nullptr;

		if (!_availablePages.empty())
		{
			pagePtr = _availablePages.front();
			_availablePages.pop();
		}
		else
		{
			pagePtr = createNewPage();
			_pagePool.emplace_back(pagePtr);
		}

		return pagePtr;
	}

	void LinearAllocatorPageManager::discardPages(uint64_t fenceValue, const eastl::vector<LinearAllocationPage*>& usedPages)
	{
		std::lock_guard<std::mutex> lockGuard(_mutex);
		for (auto it = usedPages.begin(); it != usedPages.end(); ++it)
			_retiredPages.push({ fenceValue, *it });
	}

	void LinearAllocatorPageManager::freeLargePages(uint64_t fenceValue, const eastl::vector<LinearAllocationPage*>& largePages)
	{
		std::lock_guard<std::mutex> lockGuard(_mutex);

		while (!_deletionQueue.empty() && g_commandQueues->isFenceComplete(_deletionQueue.front().first))
		{
			delete _deletionQueue.front().second;
			_deletionQueue.pop();
		}

		for (auto it = largePages.begin(); it != largePages.end(); ++it)
		{
			(*it)->unmap();
			_deletionQueue.push({ fenceValue, *it });
		}
	}

	LinearAllocationPage* LinearAllocatorPageManager::createNewPage(size_t pageSize)
	{
		D3D12_HEAP_PROPERTIES heapProps;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 0;
		heapProps.VisibleNodeMask = 0;

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

		D3D12_RESOURCE_STATES defaultUsage;

		if (_allocatorType == GpuExclusive)
		{
			LOG("GpuExclusive allocator not supported (barrier), maybe page behavior wrong.");
			_ASSERT(false);
			
			heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
			resourceDesc.Width = pageSize == 0 ? GpuAllocatorPageSize : pageSize;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			defaultUsage = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		}
		else
		{
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			resourceDesc.Width = pageSize == 0 ? CpuAllocatorPageSize : pageSize;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			defaultUsage = D3D12_RESOURCE_STATE_GENERIC_READ;
		}

		ID3D12Resource* buffer;
		auto res = g_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, 
			              &resourceDesc, defaultUsage, nullptr, IID_PPV_ARGS(&buffer));

		_ASSERT(SUCCEEDED(res));

		return new LinearAllocationPage(buffer, defaultUsage);
	}


	/*******************/
	/* LinearAllocator */
	/*******************/

	void LinearAllocator::cleanup(uint64_t fenceID)
	{
		if (_curPage == nullptr)
			return;

		_retiredPages.push_back(_curPage);
		_curPage = nullptr;
		_curOffset = 0;

		_pageManager[_allocatorType].discardPages(fenceID, _retiredPages);
		_retiredPages.clear();

		_pageManager[_allocatorType].freeLargePages(fenceID, _largePageList);
		_largePageList.clear();
	}

	DynAlloc LinearAllocator::allocateLargePage(size_t sizeInBytes)
	{
		LinearAllocationPage* newPage = _pageManager[_allocatorType].createNewPage(sizeInBytes);
		_largePageList.push_back(newPage);

		DynAlloc ret(*newPage, 0, sizeInBytes);
		ret.dataPtr = newPage->_cpuVirtualAddress;
		ret.gpuAddress = newPage->gpuVirtualAdress();

		return ret;
	}

	namespace
	{
		size_t alignUpWithMask(size_t value, size_t mask)
		{
			return (value + mask) & ~mask;
		}
	};

	DynAlloc LinearAllocator::allocate(size_t sizeInBytes, size_t alignment)
	{
		const size_t AlignmentMask = alignment - 1;

		// Assert that it's a power of two.
		_ASSERT((AlignmentMask & alignment) == 0);

		// Align the allocation
		const size_t AlignedSize = alignUpWithMask(sizeInBytes, AlignmentMask);

		if (AlignedSize > _pageSize)
			return allocateLargePage(AlignedSize);

		_curOffset = alignUpWithMask(_curOffset, AlignmentMask);

		if (_curOffset + AlignedSize > _pageSize)
		{
			_ASSERT(_curPage != nullptr);
			_retiredPages.push_back(_curPage);
			_curPage = nullptr;
		}

		if (_curPage == nullptr)
		{
			_curPage = _pageManager[_allocatorType].requestPage();
			_curOffset = 0;
		}

		DynAlloc ret(*_curPage, _curOffset, AlignedSize);
		ret.dataPtr = (uint8_t*)_curPage->_cpuVirtualAddress + _curOffset;
		ret.gpuAddress = _curPage->gpuVirtualAdress() + _curOffset;

		_curOffset += AlignedSize;

		return ret;
	}
}
