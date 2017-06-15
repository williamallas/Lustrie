#pragma once

#include "DX12.h"
#include <core/NonCopyable.h>

namespace dx12
{
	class RootSignature : NonCopyable
	{
	public:
		class RootParameter : NonCopyable
		{
			friend class RootSignature;
		public:
			RootParameter() { _param.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF; }
			~RootParameter() { clear(); }

			RootParameter(RootParameter&& param) 
			{
				_param = param._param;
				param._param.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
			}

			RootParameter& operator=(RootParameter&& param)
			{
				_param = param._param;
				param._param.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
			}

			void setAsConstants(UINT registre, UINT numWords, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
			{
				_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
				_param.ShaderVisibility = visibility;
				_param.Constants.Num32BitValues = numWords;
				_param.Constants.ShaderRegister = registre;
				_param.Constants.RegisterSpace = 0;
			}

			void setAsConstantBuffer(UINT registre, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
			{
				_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
				_param.ShaderVisibility = visibility;
				_param.Descriptor.ShaderRegister = registre;
				_param.Descriptor.RegisterSpace = 0;
			}

		private:
			D3D12_ROOT_PARAMETER _param;

			void clear()
			{
				if (_param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
					delete[] _param.DescriptorTable.pDescriptorRanges;

				_param.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
			}
		};

		RootSignature(RootSignature&&) = default;
		RootSignature& operator=(RootSignature&&) = default;

		RootSignature(ID3D12Device* device, const eastl::vector<RootParameter>& param, D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)
		{
			D3D12_ROOT_SIGNATURE_DESC rootDesc;
			rootDesc.NumParameters = param.size();
			rootDesc.pParameters = (const D3D12_ROOT_PARAMETER*)param.data();
			rootDesc.NumStaticSamplers = 0;
			rootDesc.pStaticSamplers = nullptr;
			rootDesc.Flags = flags;

			Microsoft::WRL::ComPtr<ID3DBlob> serialized, error;

			auto res = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, serialized.GetAddressOf(), error.GetAddressOf());
			_ASSERT(SUCCEEDED(res));

			res = device->CreateRootSignature(1, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&_signature));
			_ASSERT(SUCCEEDED(res));
		}

		ID3D12RootSignature* rootSignature() const { return _signature.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D12RootSignature> _signature;
	};
}