#pragma once

#include "DX12.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <EASTL/unique_ptr.h>
#include <core/NonCopyable.h>
#include <graphics/RendererStruct.h>
#include <math/Matrix.h>
#include <math/Camera.h>

#include "DX12PipelineState.h"
#include "DX12RootSignature.h"
#include "DX12CommandQueue.h"
#include "DX12CommandList.h"
#include "DX12Resource.h"
#include "DX12DescriptorAllocator.h"
#include "DX12TextureBuffer.h"

class Material;
class MeshBuffers;

namespace dx12
{
	using Shader = Microsoft::WRL::ComPtr<ID3DBlob>;

	class Renderer : NonCopyable
	{
	public:
		Renderer() = default;
		~Renderer();

		bool init(const InitRendererInfo&);

		void beginRender(const tim::Camera& camera, float timeElapsed);
		void render(const Material&, const eastl::vector<ObjectInstance>&);
		void endRender();
		void close();

		static Shader compileShader(eastl::string src, ShaderType, eastl::string entryPoint = "main");

		ID3D12Device* device() const { return _device; }

	private:
		friend class PipelineState;

		InitRendererInfo _rendererInfo;
		ID3D12Device* _device = nullptr;

		static const int NB_BUFFERS = 2;
		IDXGISwapChain3* _swapChain;
		ID3D12Resource* _backBufferRenderTarget[NB_BUFFERS];
		unsigned int _bufferIndex;

		eastl::unique_ptr<CommandQueueManager> _commandQueueManager;
		GraphicsCommandContext* _commandContext;
		Descriptor _backBufferDescriptors[NB_BUFFERS];

		DepthBuffer _depthBuffers[NB_BUFFERS];

		struct FrameConstants
		{
			tim::mat4 view;
			tim::mat4 proj;
			tim::mat4 projView;
			tim::vec4 cameraPos_time;
		};

		// persistently mapped buffer
		CpuWritableBuffer _frameConstantsBuffer[NB_BUFFERS];
		FrameConstants* _frameConstantsBufferPtr[NB_BUFFERS] = { nullptr, nullptr };

		// buffer for matrix and material, need several map unmap
		struct InstanceConstants
		{
			tim::ivec4 textures;
			tim::vec4 material;
		};

		static constexpr UINT64 MAX_INSTANCE = 1 << 14;

		CpuWritableBuffer _matrixBuffers[NB_BUFFERS];
		tim::mat4* _matrixBuffersPtr[NB_BUFFERS];

		CpuWritableBuffer _materialBuffers[NB_BUFFERS];
		InstanceConstants* _materialBuffersPtr[NB_BUFFERS];
		size_t _indexInBuffer;

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

