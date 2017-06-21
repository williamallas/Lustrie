#pragma once

#include "DX12.h"
#include "DX12CommandQueue.h"
#include <core/NonCopyable.h>
#include "DX12Buffer.h"
#include "DX12Texture.h"
#include "DX12Allocator.h"

#include <EASTL/unique_ptr.h>

namespace dx12
{
	class CommandContext : NonCopyable
	{
		friend class CommandListAllocator;

	public:
		static CommandContext& AllocContext(CommandQueue::CommandQueueType);
		static void destroyAllContexts();

		virtual ~CommandContext() = default;

		CommandQueue::CommandQueueType type() const;
		ID3D12GraphicsCommandList* commandList() const;
		LinearAllocator& cpuAllocator();

		void reset(); // give back the command allocator
		uint64_t flush(bool wait, bool gcAllocator = false);
		uint64_t finish(bool wait);

		void copyBuffer(Resource& dest, Resource& src);
		void copySubBuffer(Resource& dest, size_t destOffset, Resource& src, size_t srcOffset, size_t numBytes);

		void copyBuffer(Resource& dest, CpuWritableBuffer& src);
		void copySubBuffer(Resource& dest, size_t destOffset, CpuWritableBuffer& src, size_t srcOffset, size_t numBytes);

		void initBuffer(Resource& dest, const void* data, size_t numBytes, size_t offset = 0);
		void initTexture(Texture& dest, const eastl::vector<D3D12_SUBRESOURCE_DATA>&);

		DynAlloc allocWritableBuffer(size_t bytes);

		void resourceBarrier(Resource&, D3D12_RESOURCE_STATES, bool immediate = false);
		void flushResourceBarriers();

	protected:
		CommandQueue::CommandQueueType _queueType;
		ID3D12GraphicsCommandList* _list = nullptr;
		ID3D12CommandAllocator* _allocator = nullptr;
		LinearAllocator _cpuAllocator;

		eastl::array<D3D12_RESOURCE_BARRIER, 16> _resourceBarrierPool;
		UINT _numBarriersToFlush = 0;

	protected:
		CommandContext(CommandQueue::CommandQueueType type);
	};

	inline ID3D12GraphicsCommandList* CommandContext::commandList() const { return _list; }
	inline LinearAllocator& CommandContext::cpuAllocator() { return _cpuAllocator; }
	inline CommandQueue::CommandQueueType CommandContext::type() const { return _queueType;  }

	class GraphicsCommandContext : public CommandContext 
	{
	public:
		void setConstants(UINT rootIndex, UINT numConstants, const void* constantData);
		//void setConstants(UINT rootIndex, DWParam X);
		//void setConstants(UINT rootIndex, DWParam X, DWParam Y);
		//void setConstants(UINT rootIndex, DWParam X, DWParam Y, DWParam Z);
		//void setConstants(UINT rootIndex, DWParam X, DWParam Y, DWParam Z, DWParam W);
		void setConstantBuffer(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS gpuAdress);

	protected:
		GraphicsCommandContext() : CommandContext(CommandQueue::DIRECT) {}
	};


	/************************/
	/* CommandListAllocator */
	/************************/

	class CommandListAllocator
	{
	public:
		CommandListAllocator() = default;

		CommandContext* allocate(CommandQueue::CommandQueueType);
		void free(CommandContext*);

		void destroyAll();

	private:
		eastl::vector<eastl::unique_ptr<CommandContext> > _pool[CommandQueue::NUM_TYPE];
		eastl::queue<CommandContext*> _availableContexts[CommandQueue::NUM_TYPE];
		std::mutex _mutex;
	};

	extern CommandListAllocator g_commandContextAllocator;
}
