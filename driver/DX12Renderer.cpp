
#include "DX12Renderer.h"
#include "Chrono.h"
#include <d3dcompiler.h>
#include "core/Logger.h"

#include "DX12PipelineState.h"
#include "DX12.h"

#include <geometry\Geometry.h>

using namespace eastl;
using namespace tim;
using Microsoft::WRL::ComPtr;

namespace dx12
{
	Renderer::~Renderer()
	{
	}


	bool Renderer::init(const InitRendererInfo& info)
	{
		_rendererInfo = info;
	#ifdef _DEBUG
		enableDebugLayer();
	#endif

		// enum adapters
		auto adapters = enumGpu();
		IDXGIAdapter* selectedAdapter = adapters[0].adapter;

		for (const AdapterInfo& desc : adapters)
		{
			std::cout << desc.name << ", Usable memory:" << (desc.videoMemory / (1 << 20)) << "MB" << std::endl;
			if (eastl::string(desc.name).find("NVIDIA") != eastl::string::npos)
				selectedAdapter = desc.adapter;
		}

		// create the Direct3D 12 device
		auto result = D3D12CreateDevice(selectedAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&_device);
		if (FAILED(result))
		{
			MessageBox(_rendererInfo.windowHandle, L"Could not create a DirectX 12.0 device.  The default video card does not support DirectX 12.0.",
				L"DirectX Device Failure", MB_OK);
			return false;
		}

		g_device = _device;

		_commandQueueManager = eastl::make_unique<CommandQueueManager>(_device);
		g_commandQueues = _commandQueueManager.get();

		_commandContext = eastl::make_unique<CommandContext>(CommandQueue::DIRECT);

		// create swap chain
		createSwapChain(NB_BUFFERS, adapters, info);
		_bufferIndex = 0;

		// create render target view
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc;
			desc.NumDescriptors = NB_BUFFERS;
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			desc.NodeMask = 0;
			result = _device->CreateDescriptorHeap(&desc, __uuidof(ID3D12DescriptorHeap), (void**)&_renderTargetViewHeap);
			_ASSERT(result >= 0);

			auto renderTargetViewHandle = _renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
			UINT renderTargetViewDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			for (int i = 0; i < NB_BUFFERS; ++i)
			{
				result = _swapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&_backBufferRenderTarget[i]);
				_ASSERT(result >= 0);

				_device->CreateRenderTargetView(_backBufferRenderTarget[i], NULL, renderTargetViewHandle);
				renderTargetViewHandle.ptr += renderTargetViewDescriptorSize;

				_fenceValue[i] = 1;
			}
		}

		_rootSignature = eastl::unique_ptr<RootSignature>(new RootSignature(_device, {}));
		_pipelineState = eastl::unique_ptr<PipelineState>(new PipelineState(_device, _rootSignature->rootSignature()));

		for (const AdapterInfo& desc : adapters)
			desc.adapter->Release();

		auto mesh = tim::Geometry::generateCubeSphere(128, 1, true);
		/*mesh.addVertex({ 0.2f,0.2f,0.2f });
		mesh.addVertex({ 0.7f,0.7f,0.7f });
		mesh.addVertex({ 0.2f,0.7f,0.2f });

		mesh.addFace({ {0,1,2,0}, 3 });
		*/

		_vertexBufferTest.alloc(mesh.nbVertices(), sizeof(vec3));
		void* ptr = _vertexBufferTest.map();
		memcpy(ptr, mesh.vertexData(), sizeof(vec3)*mesh.nbVertices());
		_vertexBufferTest.unmap();

		auto indexBuffer = mesh.indexData();
		std::cout << "nb triangle : " << indexBuffer.size()/3 << std::endl;
		_indexBufferTest.alloc(indexBuffer.size(), sizeof(uint));
		ptr = _indexBufferTest.map();
		memcpy(ptr, indexBuffer.data(), sizeof(uint)*indexBuffer.size());
		_indexBufferTest.unmap();

		_vertexBufferTestGpu.alloc(mesh.nbVertices(), sizeof(vec3));
		_indexBufferTestGpu.alloc(indexBuffer.size(), sizeof(uint));

		_commandContext->copyBuffer(_vertexBufferTestGpu, _vertexBufferTest);
		_commandContext->copyBuffer(_indexBufferTestGpu, _indexBufferTest);
		_commandContext->flush(true);

		//_vertexBufferTest.clear();
		//_indexBufferTest.clear();
		
		return true;
	}


	void Renderer::render()
	{
		//DebugTimer dfdf("asdads");
		Chrono crono;

		// start recording the command list

		// add a barrier on the target buffer
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = _backBufferRenderTarget[_bufferIndex];
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		_commandContext->commandList()->ResourceBarrier(1, &barrier);
		
		// set the back buffer as the render targe
		auto renderTargetViewHandle = _renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
		UINT renderTargetViewDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		renderTargetViewHandle.ptr += renderTargetViewDescriptorSize * _bufferIndex;
		_commandContext->commandList()->OMSetRenderTargets(1, &renderTargetViewHandle, FALSE, nullptr);

		float color[2][4] = { { 0.3f, 0.5f, 1, 1 },{ 1, 0.5f, 1, 1 } };
		_commandContext->commandList()->ClearRenderTargetView(renderTargetViewHandle, color[0], 0, nullptr);

		_commandContext->commandList()->SetPipelineState(_pipelineState->getPipelineState());
		_commandContext->commandList()->SetGraphicsRootSignature(_rootSignature->rootSignature());

		CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>(_rendererInfo.resolution.x()), static_cast<float>(_rendererInfo.resolution.y()) );
		CD3DX12_RECT rect = CD3DX12_RECT( 0, 0, static_cast<LONG>(_rendererInfo.resolution.x()), static_cast<LONG>(_rendererInfo.resolution.y()) );
		_commandContext->commandList()->RSSetViewports(1, &viewport);
		_commandContext->commandList()->RSSetScissorRects(1, &rect);

		_commandContext->commandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_commandContext->commandList()->IASetVertexBuffers(0, 1, &_vertexBufferTestGpu.vertexBufferView());
		_commandContext->commandList()->IASetIndexBuffer(&_indexBufferTestGpu.indexBufferView());
		_commandContext->commandList()->DrawIndexedInstanced(_indexBufferTestGpu.elemCount(), 1, 0, 0, 0);
		_commandContext->commandList()->DrawInstanced(_vertexBufferTest.elemCount(), 1, 0, 0);
		
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		_commandContext->commandList()->ResourceBarrier(1, &barrier);

		static int numFrame = 0;
		static float frameTime = 0;

		_fenceValue[_bufferIndex] = _commandContext->flush(false, true);
		
		HRESULT result;
		if (_rendererInfo.vsync)
			result = _swapChain->Present(1, 0);
		else
			result = _swapChain->Present(0, 0);

		_ASSERT(result >= 0);

		_bufferIndex = (_bufferIndex + 1) % NB_BUFFERS;
		_commandQueueManager->waitForFence(_fenceValue[_bufferIndex]);

		frameTime += float(crono.elapsed().to_secs());
		numFrame++;
		if (numFrame % 500 == 0)
		{
			std::cout << (numFrame / frameTime) << " fps\n";
			frameTime = 0;
			numFrame = 0;
		}
	}


	namespace
	{
		template<class T> void Release(T* & obj)
		{
			if (obj)
			{
				obj->Release();
				obj = nullptr;
			}
		}
	}


	void Renderer::close()
	{
		_commandQueueManager->sync();

		if (_swapChain)
			_swapChain->SetFullscreenState(false, nullptr);

		for (int i = 0; i < 2; ++i)
			Release(_backBufferRenderTarget[i]);

		Release(_renderTargetViewHeap);
		Release(_swapChain);

		_commandQueueManager.release();
		_pipelineState.release();
		_rootSignature.release();

		Release(_device);
	}


	/* private */

	vector<Renderer::AdapterInfo> Renderer::enumGpu()
	{
		// Create a DirectX graphics interface factory.
		IDXGIFactory4* factory = nullptr;
		auto result = CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)&factory);
		if (FAILED(result))
			return {};

		UINT i = 0;
		size_t tmp;
		IDXGIAdapter* adapter;
		vector<AdapterInfo> descriptions;
		while (factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC desc;
			adapter->GetDesc(&desc);

			AdapterInfo info;
			wcstombs_s(&tmp, info.name, 128, desc.Description, 128);
			info.systemMemory = desc.DedicatedSystemMemory;
			info.videoMemory = desc.DedicatedVideoMemory;
			info.adapter = adapter;
			descriptions.push_back(info);

			++i;
		}
		factory->Release();

		return descriptions;
	}

	void Renderer::createSwapChain(int nbBuffers, const eastl::vector<AdapterInfo>& adapters, const InitRendererInfo& info)
	{
		// enumerate the primary adapter output (monitor)

		IDXGIOutput * adapterOutput = nullptr;
		for (auto adapter : adapters)
		{
			if (adapter.adapter->EnumOutputs(0, &adapterOutput) != DXGI_ERROR_NOT_FOUND)
				break;
		}

		_ASSERT(adapterOutput != nullptr);

		UINT numModes;
		auto result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
		_ASSERT(result >= 0);


		DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
		result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
		_ASSERT(result >= 0);

		// now go through all the display modes and find the one that matches the screen height and width to find the refresh rate.
		int numerator = 0, denominator = 1;
		for (UINT i = 0; i < numModes; i++)
		{
			if (displayModeList[i].Height == (UINT)info.resolution.y() && displayModeList[i].Width == (UINT)info.resolution.x())
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}

		// Create a DirectX graphics interface factory.
		IDXGIFactory4* factory = nullptr;
		auto resultFactory = CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)&factory);
		_ASSERT(result >= 0);

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

		swapChainDesc.BufferCount = nbBuffers;
		swapChainDesc.BufferDesc.Width = (UINT)info.resolution.x();
		swapChainDesc.BufferDesc.Height = (UINT)info.resolution.y();
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		if (info.vsync)
		{
			swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
		}
		else
		{
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		}

		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;

		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = info.windowHandle;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Windowed = info.fullscreen ? FALSE : TRUE;
		swapChainDesc.Flags = 0;// info.fullscreen ? 0 : DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

		IDXGISwapChain* swapChain;
		factory->CreateSwapChain(_commandQueueManager->commandQueue(CommandQueue::DIRECT).queue(), &swapChainDesc, &swapChain);
		swapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&_swapChain);
		_bufferIndex = _swapChain->GetCurrentBackBufferIndex();

		factory->Release();
		delete[] displayModeList;
		adapterOutput->Release();
	}

	void Renderer::enableDebugLayer() const
	{
		ID3D12Debug* debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			debugController->Release();
		}
		else
			_ASSERT(false);
	}

	Shader Renderer::compileShader(string src, ShaderType type, string entryPoint)
	{
		UINT compileFlags = 0;

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		ComPtr<ID3DBlob> shaderByteCode;
		ComPtr<ID3DBlob> error;

		eastl::string shaderModel;
		switch (type)
		{
		case ShaderType::VERTEX: shaderModel = "vs_5_0";
			break;
		case ShaderType::PIXEL: shaderModel = "ps_5_0";
			break;
		case ShaderType::COMPUTE: shaderModel = "cs_5_0";
			break;
		case ShaderType::GEOMETRY: shaderModel = "gs_5_0";
			break;
		}

		/*const D3D_SHADER_MACRO defines[] =
		{
			"EXAMPLE_DEFINE", "1",
			NULL, NULL
		};*/

		auto compileOk = D3DCompile(src.c_str(), src.size(), nullptr, NULL/*defines*/, NULL, entryPoint.c_str(), shaderModel.c_str(), compileFlags, 0, &shaderByteCode, &error);

		if (FAILED(compileOk))
		{
			if (error)
			{
				LOG("Error compiling: ", src, "\n---------------------------------");
				LOG((char*)error->GetBufferPointer());
				return ComPtr<ID3DBlob>();
			}
		}

		return shaderByteCode;
	}
}