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
			RootParameter() 
			{ 
				_param.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
				_param.DescriptorTable.pDescriptorRanges = nullptr;
			}

			~RootParameter() { clear(); }

			RootParameter(RootParameter&& param) 
			{
				_param = param._param;
				param._param.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
				param._param.DescriptorTable.pDescriptorRanges = nullptr;
			}

			RootParameter& operator=(RootParameter&& param)
			{
				_param = param._param;
				param._param.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
				param._param.DescriptorTable.pDescriptorRanges = nullptr;
			}

			void setAsConstants(UINT registre, UINT numWords, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
			{
				clear();
				_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
				_param.ShaderVisibility = visibility;
				_param.Constants.Num32BitValues = numWords;
				_param.Constants.ShaderRegister = registre;
				_param.Constants.RegisterSpace = 0;
			}

			void setAsConstantBuffer(UINT registre, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
			{
				clear();
				_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
				_param.ShaderVisibility = visibility;
				_param.Descriptor.ShaderRegister = registre;
				_param.Descriptor.RegisterSpace = 0;
			}

			void setAsBufferSRV(UINT registre, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
			{
				clear();
				_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
				_param.ShaderVisibility = visibility;
				_param.Descriptor.ShaderRegister = registre;
				_param.Descriptor.RegisterSpace = 0;
			}

			void setAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE type, UINT registre, UINT count, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
			{
				clear();
				setAsDescriptorTable(1, visibility);
				setRangeInTable(0, type, registre, count);
			}

			void setAsDescriptorTable(UINT rangeCount, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
			{
				clear();
				_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				_param.ShaderVisibility = visibility;
				_param.DescriptorTable.NumDescriptorRanges = rangeCount;
				_param.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[rangeCount];
			}

			void setRangeInTable(UINT rangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE type, UINT registre, UINT count, UINT space = 0)
			{
				D3D12_DESCRIPTOR_RANGE* range = const_cast<D3D12_DESCRIPTOR_RANGE*>(_param.DescriptorTable.pDescriptorRanges + rangeIndex);
				range->RangeType = type;
				range->NumDescriptors = count;
				range->BaseShaderRegister = registre;
				range->RegisterSpace = space;
				range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			}

		private:
			D3D12_ROOT_PARAMETER _param;

			void clear()
			{
				if (_param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
				{
					delete[] _param.DescriptorTable.pDescriptorRanges;
					_param.DescriptorTable.pDescriptorRanges = nullptr;
				}

				_param.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
			}
		};

		RootSignature(RootSignature&&) = default;
		RootSignature& operator=(RootSignature&&) = default;

		RootSignature(ID3D12Device* device, const eastl::vector<RootParameter>& param,
			const eastl::vector<D3D12_STATIC_SAMPLER_DESC>& samplers,
			D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ID3D12RootSignature* rootSignature() const { return _signature.Get(); }

		static D3D12_STATIC_SAMPLER_DESC linearWrapSampler(UINT reg, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_PIXEL);
		static D3D12_STATIC_SAMPLER_DESC nearestWrapSampler(UINT reg, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_PIXEL);
		static D3D12_STATIC_SAMPLER_DESC linearClampSampler(UINT reg, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_PIXEL);
		static D3D12_STATIC_SAMPLER_DESC nearestClampSampler(UINT reg, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_PIXEL);
		static D3D12_STATIC_SAMPLER_DESC trilinearWrapSampler(UINT reg, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_PIXEL);
		static D3D12_STATIC_SAMPLER_DESC trilinearClampSampler(UINT reg, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_PIXEL);

	private:
		Microsoft::WRL::ComPtr<ID3D12RootSignature> _signature;
	};
}