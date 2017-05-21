#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <core/NonCopyable.h>
#include <core/type.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>

#include "DX12InputLayout.h"
#include "DX12RootSignature.h"

namespace dx12
{
	class Renderer;

	class PipelineState : NonCopyable
	{
	public:
		PipelineState(ID3D12Device* device, ID3D12RootSignature*);

		ID3D12PipelineState* getPipelineState() const { return _pipeline.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipeline;

	private:
		static void initDefaultDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC&, const DX12InputLayout&);
	};
}