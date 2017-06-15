#pragma once

#include "DX12.h"

namespace dx12
{
	class DX12InputLayout : EANonCopyable
	{
	public:
		DX12InputLayout() = default;

		void initAsFloatVectors(const eastl::vector<eastl::pair<eastl::string, UINT>>& layout) // (name , num float)
		{
			clear();
			UINT stride = 0;

			for (auto elem : layout)
			{
				D3D12_INPUT_ELEMENT_DESC desc;
				desc.SemanticName = NULL; // setup later
				desc.SemanticIndex = 0;
				desc.AlignedByteOffset = stride; // in buffer stride

				switch (elem.second)
				{
				case 1: 
					desc.Format = DXGI_FORMAT_R32_FLOAT; 
					stride += 4;
					break;

				case 2: 
					desc.Format = DXGI_FORMAT_R32G32_FLOAT; 
					stride += 8;
					break;

				case 3: 
					desc.Format = DXGI_FORMAT_R32G32B32_FLOAT; 
					stride += 12;
					break;

				case 4: 
					desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; 
					stride += 16;
					break;

				default: 
					desc.Format = DXGI_FORMAT_R32G32B32_FLOAT; 
					stride += 12;
					break;
				}

				desc.InputSlot = 0; // vertex buffer slot
				desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				desc.InstanceDataStepRate = 0; 
				
				
				_layout.push_back(desc);
				_names.push_back(elem.first);
			}

			setupName();
		}

		UINT getNum() const { return _layout.size(); }

		const D3D12_INPUT_ELEMENT_DESC* getLayout() const { return _layout.data();  }

	private:
		eastl::vector<D3D12_INPUT_ELEMENT_DESC> _layout;
		eastl::vector<eastl::string> _names;

		void clear()
		{
			_layout.clear();
			_names.clear();
		}

		void setupName()
		{
			for (size_t i = 0; i < _layout.size(); ++i)
				_layout[i].SemanticName = _names[i].c_str();
		}
	};
}