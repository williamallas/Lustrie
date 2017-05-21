#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/unique_ptr.h>
#include <core/NonCopyable.h>
#include "RendererStruct.h"

#include "DX12PipelineState.h"
#include "DX12RootSignature.h"

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
		ID3D12CommandQueue* _commandQueue;

		IDXGISwapChain3* _swapChain;
		ID3D12DescriptorHeap* _renderTargetViewHeap;
		ID3D12Resource* _backBufferRenderTarget[2];
		unsigned int _bufferIndex;

		ID3D12CommandAllocator* _commandAllocator[2];
		ID3D12GraphicsCommandList* _commandList;

		eastl::unique_ptr<RootSignature> _rootSignature;
		eastl::unique_ptr<PipelineState> _pipelineState;

		ID3D12Fence* _fence;
		HANDLE _fenceEvent;
		unsigned long long _fenceValue[2];

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

		void moveToNextFrame();
		void waitFrame();
	};
}

