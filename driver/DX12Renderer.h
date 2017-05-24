#pragma once

#include "DX12.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <EASTL/unique_ptr.h>
#include <core/NonCopyable.h>
#include "RendererStruct.h"

#include "DX12PipelineState.h"
#include "DX12RootSignature.h"
#include "DX12CommandQueue.h"
#include "DX12CommandList.h"
#include "DX12Resource.h"

namespace dx12
{
	using Shader = Microsoft::WRL::ComPtr<ID3DBlob>;

	class Renderer : NonCopyable
	{
	public:
		Renderer() = default;
		~Renderer();

		bool init(const InitRendererInfo&);
		void render();
		void close();

		static Shader compileShader(eastl::string src, ShaderType, eastl::string entryPoint = "main");

	private:
		friend class PipelineState;

		InitRendererInfo _rendererInfo;
		ID3D12Device* _device = nullptr;

		static const int NB_BUFFERS = 2;
		IDXGISwapChain3* _swapChain;
		ID3D12DescriptorHeap* _renderTargetViewHeap;
		ID3D12Resource* _backBufferRenderTarget[NB_BUFFERS];
		unsigned int _bufferIndex;

		eastl::unique_ptr<RootSignature> _rootSignature;
		eastl::unique_ptr<PipelineState> _pipelineState;

		eastl::unique_ptr<CommandQueueManager> _commandQueueManager;
		eastl::unique_ptr<CommandContext> _commandContext;

		CpuWritableBuffer _vertexBufferTest;
		CpuWritableBuffer _indexBufferTest;
		GpuBuffer _vertexBufferTestGpu;
		GpuBuffer _indexBufferTestGpu;

		unsigned long long _fenceValue[NB_BUFFERS];

	private:
		struct AdapterInfo
		{
			char name[128];
			size_t videoMemory;
			size_t systemMemory;
			IDXGIAdapter* adapter;
		};

	private:
		static eastl::vector<AdapterInfo> enumGpu();

		void createSwapChain(int, const eastl::vector<AdapterInfo>&, const InitRendererInfo&);
		void enableDebugLayer() const;
	};
}

