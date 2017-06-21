#include "DX12RootSignature.h"

namespace dx12
{
	RootSignature::RootSignature(ID3D12Device* device, const eastl::vector<RootParameter>& param,
		const eastl::vector<D3D12_STATIC_SAMPLER_DESC>& samplers, D3D12_ROOT_SIGNATURE_FLAGS flags)
	{
		D3D12_ROOT_SIGNATURE_DESC rootDesc;
		rootDesc.NumParameters = param.size();
		rootDesc.pParameters = (const D3D12_ROOT_PARAMETER*)param.data();
		rootDesc.NumStaticSamplers = samplers.size();
		rootDesc.pStaticSamplers = samplers.data();
		rootDesc.Flags = flags;

		Microsoft::WRL::ComPtr<ID3DBlob> serialized, error;

		auto res = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), error.GetAddressOf());
		_ASSERT(SUCCEEDED(res));

		res = device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&_signature));
		_ASSERT(SUCCEEDED(res));
	}

	namespace
	{
		void setDefault(D3D12_STATIC_SAMPLER_DESC& desc)
		{
			desc.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			desc.MipLODBias = 0;
			desc.MaxAnisotropy = 0;
			desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
			desc.MinLOD = 0;
			desc.MaxLOD = D3D12_FLOAT32_MAX;
			desc.ShaderRegister = 0;
			desc.RegisterSpace = 0;
			desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		}
	};

	D3D12_STATIC_SAMPLER_DESC RootSignature::linearWrapSampler(UINT reg, D3D12_SHADER_VISIBILITY visibility)
	{
		D3D12_STATIC_SAMPLER_DESC desc;
		setDefault(desc);
		desc.ShaderRegister = reg;
		desc.ShaderVisibility = visibility;
		desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

		return desc;
	}

	D3D12_STATIC_SAMPLER_DESC RootSignature::nearestWrapSampler(UINT reg, D3D12_SHADER_VISIBILITY visibility)
	{
		D3D12_STATIC_SAMPLER_DESC desc;
		setDefault(desc);
		desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

		desc.ShaderRegister = reg;
		desc.ShaderVisibility = visibility;
		return desc;
	}

	D3D12_STATIC_SAMPLER_DESC RootSignature::linearClampSampler(UINT reg, D3D12_SHADER_VISIBILITY visibility)
	{
		D3D12_STATIC_SAMPLER_DESC desc;
		setDefault(desc);
		desc.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

		desc.ShaderRegister = reg;
		desc.ShaderVisibility = visibility;
		return desc;
	}

	D3D12_STATIC_SAMPLER_DESC RootSignature::nearestClampSampler(UINT reg, D3D12_SHADER_VISIBILITY visibility)
	{
		D3D12_STATIC_SAMPLER_DESC desc;
		setDefault(desc);
		desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

		desc.ShaderRegister = reg;
		desc.ShaderVisibility = visibility;
		return desc;
	}
}