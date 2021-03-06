#pragma once

#include "DX12.h"
#include <core/NonCopyable.h>
#include <core/type.h>


#include "DX12InputLayout.h"
#include "DX12RootSignature.h"

namespace dx12
{
	class Renderer;

	class PipelineState : NonCopyable
	{
	public:
		enum Primitive
		{
			Triangle,
			Point
		};

		struct ForwardPipelineParam
		{
			bool wireframe = false;
			bool hdr = false;
			bool cullFace = true;
			Primitive primitive = Triangle;
		};

		PipelineState(ID3D12Device* device, ID3D12RootSignature*, DX12InputLayout&, const eastl::string& shaderSrc, 
					  const ForwardPipelineParam&, bool useGS = false);

		ID3D12PipelineState* getPipelineState() const { return _pipeline.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipeline;

	private:
		static void initDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC&, const DX12InputLayout&, const ForwardPipelineParam&);

		static D3D12_PRIMITIVE_TOPOLOGY_TYPE convertToD3DType(Primitive);
	};
}