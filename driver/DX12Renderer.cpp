
#include "DX12Renderer.h"
#include "Chrono.h"
#include <d3dcompiler.h>
#include "core/Logger.h"

#include "DX12PipelineState.h"
#include "DX12.h"

using namespace eastl;
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

		// create the command queue
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDesc.NodeMask = 0;

		result = _device->CreateCommandQueue(&commandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)&_commandQueue);
		_ASSERT(result >= 0);

		// create swap chain
		const int NB_BUFFERS = 2;
		createSwapChain(NB_BUFFERS, adapters, info);

		// create render target view
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc;
			desc.NumDescriptors = NB_BUFFERS;
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			desc.NodeMask = 0;
			result = _device->CreateDescriptorHeap(&desc, __uuidof(ID3D12DescriptorHeap), (void**)&_renderTargetViewHeap);
			_ASSERT(result >= 0);

			result = _swapChain->GetBuffer(0, __uuidof(ID3D12Resource), (void**)&_backBufferRenderTarget[0]);
			_ASSERT(result >= 0);
			result = _swapChain->GetBuffer(1, __uuidof(ID3D12Resource), (void**)&_backBufferRenderTarget[1]);
			_ASSERT(result >= 0);

			auto renderTargetViewHandle = _renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
			UINT renderTargetViewDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			_device->CreateRenderTargetView(_backBufferRenderTarget[0], NULL, renderTargetViewHandle);
			renderTargetViewHandle.ptr += renderTargetViewDescriptorSize;
			_device->CreateRenderTargetView(_backBufferRenderTarget[1], NULL, renderTargetViewHandle);
		}

		_rootSignature = eastl::unique_ptr<RootSignature>(new RootSignature(_device, {}));
		_pipelineState = eastl::unique_ptr<PipelineState>(new PipelineState(_device, _rootSignature->rootSignature()));

		// create command allocator and command list
		for (int i = 0; i < 2; ++i)
		{
			result = _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&_commandAllocator[i]);
			_ASSERT(result >= 0);
		}

		result = _device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator[_bufferIndex], NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&_commandList);
		_ASSERT(result >= 0);
		result = _commandList->Close();
		_ASSERT(result >= 0);

		// create fence
		result = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&_fence);
		_ASSERT(result >= 0);

		// create an event object for the fence.
		_fenceEvent = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
		_ASSERT(_fenceEvent != NULL);
		_fenceValue[0] = _fenceValue[1] = 1;

		for (const AdapterInfo& desc : adapters)
			desc.adapter->Release();

		return true;
	}


	void Renderer::render()
	{
		DebugTimer timer("Frame");

		auto result = _commandAllocator[_bufferIndex]->Reset();
		_ASSERT(result >= 0);

		result = _commandList->Reset(_commandAllocator[_bufferIndex], nullptr);
		_ASSERT(result >= 0);

		// start recording the command list

		// add a barrier on the target buffer
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = _backBufferRenderTarget[_bufferIndex];
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		_commandList->ResourceBarrier(1, &barrier);

		// set the back buffer as the render targe
		auto renderTargetViewHandle = _renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();
		UINT renderTargetViewDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		renderTargetViewHandle.ptr += renderTargetViewDescriptorSize * _bufferIndex;
		_commandList->OMSetRenderTargets(1, &renderTargetViewHandle, FALSE, nullptr);

		float color[2][4] = { { 0.3f, 0.5f, 1, 1 },{ 1, 0.5f, 1, 1 } };
		_commandList->ClearRenderTargetView(renderTargetViewHandle, color[0], 0, nullptr);

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		_commandList->ResourceBarrier(1, &barrier);

		result = _commandList->Close();
		_ASSERT(result >= 0);

		ID3D12CommandList* commandLists[1];
		commandLists[0] = _commandList;
		_commandQueue->ExecuteCommandLists(1, commandLists);

		if (_rendererInfo.vsync)
			_swapChain->Present(1, 0);
		else
			result = _swapChain->Present(0, 0);
		_ASSERT(result >= 0);

		moveToNextFrame();
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
		waitFrame();

		if (_swapChain)
			_swapChain->SetFullscreenState(false, nullptr);

		CloseHandle(_fenceEvent);

		Release(_fence);
		Release(_commandList);
		for (int i = 0; i < 2; ++i)
			Release(_commandAllocator[i]);

		Release(_fence);

		for (int i = 0; i < 2; ++i)
			Release(_backBufferRenderTarget[i]);

		Release(_renderTargetViewHeap);
		Release(_swapChain);
		Release(_commandQueue);
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
		factory->CreateSwapChain(_commandQueue, &swapChainDesc, &swapChain);
		swapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&_swapChain);
		_bufferIndex = _swapChain->GetCurrentBackBufferIndex();

		factory->Release();
		delete[] displayModeList;
		adapterOutput->Release();
	}

	void Renderer::moveToNextFrame()
	{
		const auto fenceValue = _fenceValue[_bufferIndex];

		// ask the queue to signal the fence when finished rendering
		auto result = _commandQueue->Signal(_fence, fenceValue);
		_ASSERT(result >= 0);

		_bufferIndex = (_bufferIndex + 1) % 2;

		// wait until the GPU is done rendering
		if (_fence->GetCompletedValue() < _fenceValue[_bufferIndex])
		{
			result = _fence->SetEventOnCompletion(_fenceValue[_bufferIndex], _fenceEvent);
			_ASSERT(result >= 0);
			WaitForSingleObject(_fenceEvent, INFINITE);
		}

		_fenceValue[_bufferIndex] = fenceValue + 1;
	}

	void Renderer::waitFrame()
	{
		// ask the queue to signal the fance when finished
		auto result = _commandQueue->Signal(_fence, _fenceValue[_bufferIndex]);
		_ASSERT(result >= 0);

		// wait until the fence is signaled
		result = _fence->SetEventOnCompletion(_fenceValue[_bufferIndex], _fenceEvent);
		_ASSERT(result >= 0);

		WaitForSingleObject(_fenceEvent, INFINITE);

		_fenceValue[_bufferIndex]++;
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