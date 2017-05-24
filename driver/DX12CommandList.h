#pragma once

#include "DX12.h"
#include "DX12CommandQueue.h"
#include <core/NonCopyable.h>
#include "DX12Resource.h"

namespace dx12
{
	class CommandContext : NonCopyable
	{
	public:
		CommandContext(CommandQueue::CommandQueueType type);
		virtual ~CommandContext() = default;

		ID3D12GraphicsCommandList* commandList() const;

		uint64_t flush(bool wait, bool gcAllocator = false);

		void copyBuffer(Resource& dest, Resource& src);
		void copySubBuffer(Resource& dest, size_t destOffset, Resource& src, size_t srcOffset, size_t numBytes);

		void copyBuffer(Resource& dest, CpuWritableBuffer& src);
		void copySubBuffer(Resource& dest, size_t destOffset, CpuWritableBuffer& src, size_t srcOffset, size_t numBytes);

		void resourceBarrier(Resource&, D3D12_RESOURCE_STATES, bool immediate = false);
		void flushResourceBarriers();

	protected:
		CommandQueue::CommandQueueType _queueType;
		ID3D12GraphicsCommandList* _list = nullptr;
		ID3D12CommandAllocator* _allocator = nullptr;

		eastl::array<D3D12_RESOURCE_BARRIER, 16> _resourceBarrierPool;
		UINT _numBarriersToFlush = 0;
	};

	inline ID3D12GraphicsCommandList* CommandContext::commandList() const { return _list; }
}
