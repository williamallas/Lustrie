#include "DX12CommandList.h"

namespace dx12
{
	CommandContext::CommandContext(CommandQueue::CommandQueueType type) : _queueType(type)
	{
		auto list_alloc = g_commandQueues->createCommandList(type);
		_list = list_alloc.first;
		_allocator = list_alloc.second;
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
		//flushResourceBarriers();
		_list->CopyBufferRegion(dest.resource(), destOffset, src.resource(), srcOffset, numBytes);
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

}