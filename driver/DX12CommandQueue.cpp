
#include "DX12CommandQueue.h"
#include <iostream>

namespace dx12
{
	/*********************/
	/*   CommandQueue    */
	/*********************/

	namespace
	{
		D3D12_COMMAND_LIST_TYPE toDx(CommandQueue::CommandQueueType type)
		{
			switch (type)
			{
			case CommandQueue::DIRECT:
				return D3D12_COMMAND_LIST_TYPE_DIRECT;
			case CommandQueue::COPY:
				return D3D12_COMMAND_LIST_TYPE_COPY;
			case CommandQueue::COMPUTE:
				return D3D12_COMMAND_LIST_TYPE_COMPUTE;
			default:
				return D3D12_COMMAND_LIST_TYPE_DIRECT;
			}
		}
	}

	CommandQueue::CommandQueue(ID3D12Device* device, CommandQueueType type) : _type(type)
	{
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
		commandQueueDesc.Type = toDx(type);
		commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDesc.NodeMask = 0;

		auto result = device->CreateCommandQueue(&commandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)_queue.GetAddressOf());
		_ASSERT(result >= 0);

		result = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
		_fence->Signal((uint64_t)_type << FENCE_OFFSET);
		
		_lastCompletedFenceValue = (uint64_t)_type << FENCE_OFFSET;
		_nextFenceValue = _lastCompletedFenceValue + 1;

		_fenceHandleEvent = CreateEvent(nullptr, false, false, nullptr);
		_ASSERT(_fenceHandleEvent != INVALID_HANDLE_VALUE);

		_allocatorPool.type = type;
		_allocatorPool.device = device;
	}

	CommandQueue::~CommandQueue()
	{
		CloseHandle(_fenceHandleEvent);
	}

	uint64_t CommandQueue::execute(ID3D12CommandList* list)
	{
		std::lock_guard<std::mutex> guard(_fenceMutex);
		_queue->ExecuteCommandLists(1, &list);
		_queue->Signal(_fence.Get(), _nextFenceValue);
		return _nextFenceValue++;
	}

	uint64_t CommandQueue::incrementFence()
	{
		std::lock_guard<std::mutex> guard(_fenceMutex);
		_queue->Signal(_fence.Get(), _nextFenceValue);
		return _nextFenceValue++;
	}

	bool CommandQueue::isFenceComplete(uint64_t value)
	{
		if (value > _lastCompletedFenceValue)
			_lastCompletedFenceValue = eastl::max(_lastCompletedFenceValue, _fence->GetCompletedValue());

		return value <= _lastCompletedFenceValue;
	}

	void CommandQueue::waitForFence(uint64_t value)
	{
		if (isFenceComplete(value))
			return;

		std::lock_guard<std::mutex> guard(_eventMutex);

		_fence->SetEventOnCompletion(value, _fenceHandleEvent);
		WaitForSingleObject(_fenceHandleEvent, INFINITE);
		_lastCompletedFenceValue = value;
	}

	void CommandQueue::sync()
	{
		waitForFence(_nextFenceValue - 1);
	}

	void CommandQueue::discardCommandAllocator(ID3D12CommandAllocator* allocator, uint64_t fenceValue)
	{
		_allocatorPool.discard(fenceValue, allocator);
	}


	/**********************/
	/* QueueAllocatorPool */
	/**********************/

	ID3D12CommandAllocator* CommandQueue::QueueAllocatorPool::get(uint64_t completedFenceValue)
	{
		std::lock_guard<std::mutex> guard(poolMutex);

		ID3D12CommandAllocator* allocator = nullptr;

		if (!readyAllocator.empty())
		{
			eastl::pair<uint64_t, ID3D12CommandAllocator*>& allocatorPair = readyAllocator.front();

			if (allocatorPair.first <= completedFenceValue)
			{
				allocator = allocatorPair.second;
				auto res = allocator->Reset();
				_ASSERT(SUCCEEDED(res));
				readyAllocator.pop();
			}
		}

		// If no allocator's were ready to be reused, create a new one
		if (allocator == nullptr)
		{
			auto res = device->CreateCommandAllocator(toDx(type), IID_PPV_ARGS(&allocator));
			_ASSERT(SUCCEEDED(res));
			allocatorPool.push_back(allocator);
		}

		return allocator;
	}

	void CommandQueue::QueueAllocatorPool::discard(uint64_t fenceValue, ID3D12CommandAllocator* allocator)
	{
		std::lock_guard<std::mutex> guard(poolMutex);
		readyAllocator.push(eastl::make_pair(fenceValue, allocator));
	}


	/*********************/
	/*   QueueManager    */
	/*********************/

	CommandQueueManager::CommandQueueManager(ID3D12Device* device)
		: _device(device), _directQueue(device, CommandQueue::DIRECT), _copyQueue(device, CommandQueue::COPY), _computeQueue(device, CommandQueue::COMPUTE)
	{
#ifdef _DEBUG
		//device->SetStablePowerState(true);
#endif
	}

	eastl::pair<ID3D12GraphicsCommandList*, ID3D12CommandAllocator*> CommandQueueManager::createCommandList(CommandQueue::CommandQueueType type)
	{
		ID3D12GraphicsCommandList* list;
		ID3D12CommandAllocator* allocator = commandQueue(type).getCommandAllocator();
		auto res = _device->CreateCommandList(1, toDx(type), allocator, nullptr, IID_PPV_ARGS(&list));
		_ASSERT(SUCCEEDED(res));

		return { list, allocator };
	}


	ID3D12CommandAllocator* CommandQueueManager::createCommandAllocator(CommandQueue::CommandQueueType type)
	{
		return commandQueue(type).getCommandAllocator();
	}
}