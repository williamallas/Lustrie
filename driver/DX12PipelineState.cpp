
#include "DX12PipelineState.h"
#include "DX12Renderer.h"
#include "DX12.h"
#include <d3dx12.h>

using Microsoft::WRL::ComPtr;

namespace dx12
{
	PipelineState::PipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature)
	{
		DX12InputLayout inLayout;
		inLayout.initAsFloatVectors({ { "VERTEX", 3 } });

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		initDefaultDesc(desc, inLayout);

		desc.pRootSignature = rootSignature;

		Shader vShader = Renderer::compileShader(g_shaderSrc, ShaderType::VERTEX, "vs_main");
		Shader pShader = Renderer::compileShader(g_shaderSrc, ShaderType::PIXEL, "ps_main");

		if (vShader)
		{
			desc.VS.pShaderBytecode = reinterpret_cast<UINT8*>(vShader->GetBufferPointer());
			desc.VS.BytecodeLength = vShader->GetBufferSize();
		}

		if (pShader)
		{
			desc.PS.pShaderBytecode = reinterpret_cast<UINT8*>(pShader->GetBufferPointer());
			desc.PS.BytecodeLength = pShader->GetBufferSize();
		}

		auto res = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(_pipeline.GetAddressOf()));
		_ASSERT(SUCCEEDED(res));
	}

	void PipelineState::initDefaultDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, const DX12InputLayout& inputLayout)
	{
		desc.pRootSignature = NULL;
		desc.DepthStencilState.StencilEnable = FALSE;
		desc.DepthStencilState.DepthEnable = FALSE;
		
		desc.InputLayout.NumElements = inputLayout.getNum();
		desc.InputLayout.pInputElementDescs = inputLayout.getLayout();

		ZeroMemory(&desc.StreamOutput, sizeof(desc.StreamOutput));
		ZeroMemory(&desc.CachedPSO, sizeof(desc.CachedPSO));
		ZeroMemory(&desc.DS, sizeof(desc.DS));
		ZeroMemory(&desc.VS, sizeof(desc.VS));
		ZeroMemory(&desc.HS, sizeof(desc.HS));
		ZeroMemory(&desc.PS, sizeof(desc.PS));
		ZeroMemory(&desc.GS, sizeof(desc.GS));
		desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		desc.NodeMask = 0;
		
		desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		desc.SampleMask = UINT_MAX;

		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

		desc.SampleDesc.Count = 1; // no msaa
		desc.SampleDesc.Quality = 0;

		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		desc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		desc.RasterizerState.FrontCounterClockwise = FALSE;
		desc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		desc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		desc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		desc.RasterizerState.DepthClipEnable = TRUE;
		desc.RasterizerState.MultisampleEnable = FALSE;
		desc.RasterizerState.AntialiasedLineEnable = FALSE;
		desc.RasterizerState.ForcedSampleCount = 0;
		desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	
	}
}