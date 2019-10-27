
#include "DX12PipelineState.h"
#include "DX12Renderer.h"
#include "DX12.h"
#include <d3dx12.h>

using Microsoft::WRL::ComPtr;

namespace dx12
{
	PipelineState::PipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature, DX12InputLayout& inLayout, 
		const eastl::string& shaderSrc, const ForwardPipelineParam& param, bool useGS)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		initDesc(desc, inLayout, param);

		desc.pRootSignature = rootSignature;

		Shader vShader = Renderer::compileShader(shaderSrc, ShaderType::VERTEX, "vs_main");
		Shader pShader = Renderer::compileShader(shaderSrc, ShaderType::PIXEL, "ps_main");
		Shader gShader;
		if (useGS)
		{
			gShader = Renderer::compileShader(shaderSrc, ShaderType::GEOMETRY, "gs_main");
			if (gShader)
			{
				desc.GS.pShaderBytecode = reinterpret_cast<UINT8*>(gShader->GetBufferPointer());
				desc.GS.BytecodeLength = gShader->GetBufferSize();
			}
		}

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

	void PipelineState::initDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, const DX12InputLayout& inputLayout, const ForwardPipelineParam& param)
	{
		desc.pRootSignature = NULL;
		desc.DepthStencilState.StencilEnable = FALSE;
		desc.DepthStencilState.DepthEnable = TRUE; 
		desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		
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
		desc.RTVFormats[0] = param.hdr ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;

		desc.SampleDesc.Count = 1; // no msaa
		desc.SampleDesc.Quality = 0;

		desc.PrimitiveTopologyType = convertToD3DType(param.primitive);

		desc.RasterizerState.FillMode = param.wireframe ? D3D12_FILL_MODE_WIREFRAME:D3D12_FILL_MODE_SOLID;
		desc.RasterizerState.CullMode = param.cullFace ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE;
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

	D3D12_PRIMITIVE_TOPOLOGY_TYPE PipelineState::convertToD3DType(Primitive primitive)
	{
		switch (primitive)
		{
		case Triangle:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		case Point:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		default:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		}
	}
}