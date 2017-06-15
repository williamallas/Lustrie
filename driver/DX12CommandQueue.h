#pragma once

#include "DX12.h"
#include <core/NonCopyable.h>
#include <mutex>
#include <EASTL/queue.h>

namespace dx12
{
	class CommandQueue : NonCopyable
	{
		friend class CommandQueueManager;
	public:
		enum  CommandQueueType { DIRECT=0, COPY=1, COMPUTE=2, NUM_TYPE = 3 };

		CommandQueue(ID3D12Device*, CommandQueueType);
		~CommandQueue();

		ID3D12CommandQueue* queue() const;

		uint64_t execute(ID3D12CommandList*);

		ID3D12CommandAllocator* getCommandAllocator();
		void discardCommandAllocator(ID3D12CommandAllocator* allocator, uint64_t fenceValue);

		uint64_t incrementFence();
		bool isFenceComplete(uint64_t);
		void waitForFence(uint64_t);
		void sync();

	private:
		const static uint64_t FENCE_OFFSET = 56;
		const CommandQueueType _type;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> _queue;

		Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
		uint64_t _nextFenceValue;
		uint64_t _lastCompletedFenceValue;
		HANDLE _fenceHandleEvent;

		std::mutex _fenceMutex, _eventMutex;

		struct QueueAllocatorPool
		{
			eastl::vector<ID3D12CommandAllocator*> allocatorPool;
			eastl::queue<eastl::pair<uint64_t, ID3D12CommandAllocator*>> readyAllocator;
			std::mutex poolMutex;
			CommandQueueType type;
			ID3D12Device* device;

			ID3D12CommandAllocator* get(uint64_t fenceValue);
			void discard(uint64_t fenceValue, ID3D12CommandAllocator*);
			~QueueAllocatorPool();
		};

		QueueAllocatorPool _allocatorPool;
	};

	inline ID3D12CommandAllocator* CommandQueue::getCommandAllocator() { return _allocatorPool.get(_fence->GetCompletedValue());  }
	inline ID3D12CommandQueue* CommandQueue::queue() const { return _queue.Get(); }
	inline CommandQueue::QueueAllocatorPool::~QueueAllocatorPool() { for (auto ptr : allocatorPool) ptr->Release(); }

	class CommandQueueManager : NonCopyable
	{
	public:
		CommandQueueManager(ID3D12Device*);
		~CommandQueueManager() = default;

		CommandQueue& commandQueue(CommandQueue::CommandQueueType type = CommandQueue::DIRECT);

		bool isFenceComplete(uint64_t value);
		void waitForFence(uint64_t);
		void sync();

		eastl::pair<ID3D12GraphicsCommandList*, ID3D12CommandAllocator*> createCommandList(CommandQueue::CommandQueueType);
		ID3D12CommandAllocator* createCommandAllocator(CommandQueue::CommandQueueType);

	private:
		ID3D12Device* _device;
		CommandQueue _directQueue;
		CommandQueue _copyQueue;
		CommandQueue _computeQueue;
	};

	inline CommandQueue& CommandQueueManager::commandQueue(CommandQueue::CommandQueueType type)
	{
		switch (type)
		{
		case CommandQueue::DIRECT:
			return _directQueue;
		case CommandQueue::COMPUTE:
			return _computeQueue;
		case CommandQueue::COPY:
			return _copyQueue;
		default:
			return _directQueue;
		}
	}

	inline bool CommandQueueManager::isFenceComplete(uint64_t value)
	{
		return commandQueue(CommandQueue::CommandQueueType(value >> CommandQueue::FENCE_OFFSET)).isFenceComplete(value);
	}

	inline void CommandQueueManager::waitForFence(uint64_t value)
	{
		commandQueue(CommandQueue::CommandQueueType(value >> CommandQueue::FENCE_OFFSET)).waitForFence(value);
	}

	inline void CommandQueueManager::sync()
	{
		_directQueue.sync();
		_computeQueue.sync();
		_copyQueue.sync();
	}
}