#include "DX12CommandList.h"

namespace dx12
{
	CommandContext::CommandContext(CommandQueue::CommandQueueType type) : _queueType(type), _cpuAllocator(LinearAllocatorType::CpuWritable)
	{
		auto list_alloc = g_commandQueues->createCommandList(type);
		_list = list_alloc.first;
		_allocator = list_alloc.second;
	}

	void CommandContext::reset()
	{
		_ASSERT(_list != nullptr && _allocator == nullptr);

		_allocator = g_commandQueues->commandQueue(_queueType).getCommandAllocator();
		_list->Reset(_allocator, nullptr);
		_numBarriersToFlush = 0;
	}

	uint64_t CommandContext::flush(bool wait, bool gcAllocator)
	{
		_ASSERT(_allocator != nullptr);

		auto result = _list->Close();
		_ASSERT(result >= 0);

		uint64_t fence = g_commandQueues->commandQueue(_queueType).execute(_list);

		if (wait)
			g_commandQueues->commandQueue(_queueType).waitForFence(fence);

		if (gcAllocator)
		{
			g_commandQueues->commandQueue(_queueType).discardCommandAllocator(_allocator, fence);
			_allocator = g_commandQueues->commandQueue(_queueType).getCommandAllocator();
		}

		_list->Reset(_allocator, nullptr);

		return fence;
	}

	uint64_t CommandContext::finish(bool wait)
	{
		_ASSERT(_allocator != nullptr);

		auto result = _list->Close();
		_ASSERT(result >= 0);

		uint64_t fence = g_commandQueues->commandQueue(_queueType).execute(_list);

		if (wait)
			g_commandQueues->commandQueue(_queueType).waitForFence(fence);

		g_commandQueues->commandQueue(_queueType).discardCommandAllocator(_allocator, fence);
		_allocator = nullptr;

		_cpuAllocator.cleanup(fence);

		g_commandContextAllocator.free(this);

		return fence;
	}

	void CommandContext::copyBuffer(Resource& dest, Resource& src)
	{
		resourceBarrier(dest, D3D12_RESOURCE_STATE_COPY_DEST);
		resourceBarrier(src, D3D12_RESOURCE_STATE_COPY_SOURCE);
		flushResourceBarriers();
		_list->CopyResource(dest.resource(), src.resource());
	}

	void CommandContext::copySubBuffer(Resource& dest, size_t destOffset, Resource& src, size_t srcOffset, size_t numBytes)
	{
		resourceBarrier(dest, D3D12_RESOURCE_STATE_COPY_DEST);
		resourceBarrier(src, D3D12_RESOURCE_STATE_COPY_SOURCE); // maybe useless
		flushResourceBarriers();
		_list->CopyBufferRegion(dest.resource(), destOffset, src.resource(), srcOffset, numBytes);
	}

	void CommandContext::copyBuffer(Resource& dest, CpuWritableBuffer& src)
	{
		resourceBarrier(dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
		//resourceBarrier(src, D3D12_RESOURCE_STATE_COPY_SOURCE);
		_list->CopyResource(dest.resource(), src.resource());
	}

	void CommandContext::copySubBuffer(Resource& dest, size_t destOffset, CpuWritableBuffer& src, size_t srcOffset, size_t numBytes)
	{
		resourceBarrier(dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
		//resourceBarrier(src, D3D12_RESOURCE_STATE_COPY_SOURCE); // maybe useless
		_list->CopyBufferRegion(dest.resource(), destOffset, src.resource(), srcOffset, numBytes);
	}

	void CommandContext::initBuffer(Resource& dest, const void* data, size_t numBytes, size_t offset)
	{
		DynAlloc alloc = _cpuAllocator.allocate(numBytes);
		memcpy(alloc.dataPtr, data, numBytes);

		resourceBarrier(dest, D3D12_RESOURCE_STATE_COPY_DEST, true);
		_list->CopyBufferRegion(dest.resource(), offset, alloc.buffer.resource(), alloc.inBufferOffset, numBytes);
	}

	void  CommandContext::initTexture(Texture& dest, const eastl::vector<D3D12_SUBRESOURCE_DATA>& subresource)
	{
		UINT64 uploadBufferSize = GetRequiredIntermediateSize(dest.resource(), 0, subresource.size());

		DynAlloc mem = _cpuAllocator.allocate((size_t)uploadBufferSize);
		UpdateSubresources(_list, dest.resource(), mem.buffer.resource(), 0, 0, (UINT)subresource.size(), const_cast<D3D12_SUBRESOURCE_DATA*>(&subresource[0]));
	}

	DynAlloc CommandContext::allocWritableBuffer(size_t numBytes)
	{
		return _cpuAllocator.allocate(numBytes);
	}

	void CommandContext::resourceBarrier(Resource& resource, D3D12_RESOURCE_STATES newState, bool immediate)
	{
		D3D12_RESOURCE_STATES oldState = resource._currentState;

		/*if (m_Type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
		{
			ASSERT((OldState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == OldState);
			ASSERT((NewState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == NewState);
		}*/

		if (oldState != newState)
		{
			_ASSERT(_numBarriersToFlush < 16); // uncommited barrier limit

			D3D12_RESOURCE_BARRIER& desc = _resourceBarrierPool[_numBarriersToFlush++];

			desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			desc.Transition.pResource = resource.resource();
			desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			desc.Transition.StateBefore = oldState;
			desc.Transition.StateAfter = newState;
			desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

			resource._currentState = newState;
		}
		/*else if (NewState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
			InsertUAVBarrier(Resource, FlushImmediate);*/

		if (immediate || _numBarriersToFlush == 16)
			flushResourceBarriers();
	}

	void CommandContext::flushResourceBarriers()
	{
		if (_numBarriersToFlush > 0)
		{
			_list->ResourceBarrier(_numBarriersToFlush, _resourceBarrierPool.data());
			_numBarriersToFlush = 0;
		}
	}

	CommandContext& CommandContext::AllocContext(CommandQueue::CommandQueueType type)
	{
		return *g_commandContextAllocator.allocate(type);
	}

	void CommandContext::destroyAllContexts()
	{
		g_commandContextAllocator.destroyAll();
	}

	/*****************************/
	/*  GraphicsCommmandContext  */
	/*****************************/

	void GraphicsCommandContext::setConstants(UINT rootIndex, UINT numConstants, const void* constantData)
	{
		_list->SetComputeRoot32BitConstants(rootIndex, numConstants, constantData, 0);
	}

	void GraphicsCommandContext::setConstantBuffer(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS gpuAdress)
	{
		_list->SetGraphicsRootConstantBufferView(rootIndex, gpuAdress);
	}

	/**************************/
	/* CommandListAllocator   */
	/**************************/

	CommandListAllocator g_commandContextAllocator;

	CommandContext* CommandListAllocator::allocate(CommandQueue::CommandQueueType type)
	{
		std::lock_guard<std::mutex> guard(_mutex);

		CommandContext* ret = nullptr;
		if (_availableContexts[type].empty()) // no available list
		{
			ret = new CommandContext(type);
			_pool[type].emplace_back(ret);
		}
		else
		{
			ret = _availableContexts[type].front();
			_availableContexts[type].pop();
			ret->reset();
		}
		return ret;
	}

	void CommandListAllocator::free(CommandContext* context)
	{
		std::lock_guard<std::mutex> guard(_mutex);
		_availableContexts[context->type()].push(context);
	}

	void CommandListAllocator::destroyAll()
	{
		for (int i = 0; i < CommandQueue::NUM_TYPE; ++i)
			_pool[i].clear();
	}
}