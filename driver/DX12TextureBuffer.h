#pragma once

#include "DX12Resource.h"
#include "DX12DescriptorAllocator.h"
#include <math/Vector.h>

namespace dx12
{
	// Texture buffer resource
	class TextureBuffer : public Resource
	{
	public:
		TextureBuffer();

		DXGI_FORMAT format() const;
		const tim::uivec3& size() const;

		static size_t bitsPerPixel(DXGI_FORMAT);

	protected:
		tim::uivec3 _size;
		tim::uint _numMips;
		DXGI_FORMAT _format;

		D3D12_RESOURCE_DESC describeTex2D(tim::uivec2 size, uint32_t depthOrArraySize, uint32_t numMips, DXGI_FORMAT format, UINT flags);

		void create(ID3D12Device*, const D3D12_RESOURCE_DESC&, const D3D12_CLEAR_VALUE* clearVal = nullptr, 
								   D3D12_GPU_VIRTUAL_ADDRESS gpuPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

		static DXGI_FORMAT getBaseFormat(DXGI_FORMAT);
		static DXGI_FORMAT getDepthFormat(DXGI_FORMAT);
		static DXGI_FORMAT getDSVFormat(DXGI_FORMAT);
	};

	inline DXGI_FORMAT TextureBuffer::format() const { return _format; }
	inline const tim::uivec3& TextureBuffer::size() const { return _size; }

	// Depth buffer
	class DepthBuffer : public TextureBuffer
	{
	public:
		DepthBuffer(float clearDepth = 1) : TextureBuffer(), _clearDepth(clearDepth) {}

		void create(tim::uivec2 res, DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT);

		const Descriptor&  depthDSV() const;
		const Descriptor&  readonlyDepthDSV() const;

		float clearDepth() const;

		void fillDepthDSVDescritpor(DXGI_FORMAT format, const Descriptor& descr) const;

	private:
		void fillDerivedViews(ID3D12Device* device, DXGI_FORMAT format);

		float _clearDepth;
		//uint8_t _clearStencil;

		Descriptor _DSV_descritpor[2]; // default DSV, depth readonly, //stencil readonly, DSV readonly
	};

	inline const Descriptor& DepthBuffer::depthDSV() const { return _DSV_descritpor[0]; }
	inline const Descriptor& DepthBuffer::readonlyDepthDSV() const { return _DSV_descritpor[1]; }

	inline float DepthBuffer::clearDepth() const { return _clearDepth; }
}

