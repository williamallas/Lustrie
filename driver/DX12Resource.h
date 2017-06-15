#pragma once

#include "DX12.h"
#include <core/NonCopyable.h>

namespace dx12
{
	// Represent any resources
	class BaseResource : NonCopyable
	{
		friend class CommandContext;
	public:
		BaseResource(ID3D12Resource* resource = nullptr, D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAdress = D3D12_GPU_VIRTUAL_ADDRESS_NULL)
			: _resource(resource), _gpuVirtualAdress(gpuVirtualAdress),
			_currentState(D3D12_RESOURCE_STATE_COMMON) {}

		virtual ~BaseResource() = default;

		BaseResource(BaseResource&&) = default;
		BaseResource& operator=(BaseResource&&) = default;

		ID3D12Resource* resource() const;
		D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAdress() const;

	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource> _resource;
		D3D12_GPU_VIRTUAL_ADDRESS _gpuVirtualAdress;
		D3D12_RESOURCE_STATES _currentState;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView(uint32_t numVertices, uint32_t stride, uint32_t offset) const;
		D3D12_INDEX_BUFFER_VIEW indexBufferView(uint32_t numIndices, uint32_t offset, bool is32bit = true) const;
	};

	inline ID3D12Resource* BaseResource::resource() const { return _resource.Get(); }
	inline D3D12_GPU_VIRTUAL_ADDRESS BaseResource::gpuVirtualAdress() const { return _gpuVirtualAdress; }

	// Represent GPU only resources
	class Resource : public BaseResource
	{
	public:
		Resource(ID3D12Resource* resource = nullptr, D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAdress = D3D12_GPU_VIRTUAL_ADDRESS_NULL)
			: BaseResource(resource, gpuVirtualAdress) {}

		Resource(Resource&&) = default;
		Resource& operator=(Resource&&) = default;
	};
}

