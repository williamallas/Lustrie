#include "DX12TextureBuffer.h"
#include "DX12.h"
#include <core/Logger.h>

namespace dx12
{
	/********************/
	/*   TextureBuffer  */
	/********************/

	TextureBuffer::TextureBuffer() : _format(DXGI_FORMAT_UNKNOWN) {}

	D3D12_RESOURCE_DESC TextureBuffer::describeTex2D(tim::uivec2 size, uint32_t depthOrArraySize, uint32_t numMips, DXGI_FORMAT format, UINT flags)
	{
		_size.x() = size.x();
		_size.y() = size.y();
		_size.z() = depthOrArraySize;
		_format = format;

		D3D12_RESOURCE_DESC Desc = {};
		Desc.Alignment = 0;
		Desc.DepthOrArraySize = (UINT16)depthOrArraySize;
		Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		Desc.Flags = (D3D12_RESOURCE_FLAGS)flags;
		Desc.Format = getBaseFormat(format);
		Desc.Height = (UINT)size.y();
		Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		Desc.MipLevels = (UINT16)numMips;
		Desc.SampleDesc.Count = 1;
		Desc.SampleDesc.Quality = 0;
		Desc.Width = (UINT64)size.x();
		return Desc;
	}

	void TextureBuffer::create(ID3D12Device* device, const D3D12_RESOURCE_DESC& desc, D3D12_CLEAR_VALUE clearValue, D3D12_GPU_VIRTUAL_ADDRESS)
	{
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		auto ret = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, 
												   &clearValue, IID_PPV_ARGS(_resource.GetAddressOf()));

		_currentState = D3D12_RESOURCE_STATE_COMMON;
		_gpuVirtualAdress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;

	}

	DXGI_FORMAT TextureBuffer::getBaseFormat(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_TYPELESS;

		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return DXGI_FORMAT_B8G8R8A8_TYPELESS;

		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return DXGI_FORMAT_B8G8R8X8_TYPELESS;

			// 32-bit Z w/ Stencil
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return DXGI_FORMAT_R32G8X24_TYPELESS;

			// No Stencil
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
			return DXGI_FORMAT_R32_TYPELESS;

			// 24-bit Z
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return DXGI_FORMAT_R24G8_TYPELESS;

			// 16-bit Z w/o Stencil
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
			return DXGI_FORMAT_R16_TYPELESS;

		default:
			return format;
		}
	}

	DXGI_FORMAT TextureBuffer::getDepthFormat(DXGI_FORMAT format)
	{
		switch (format)
		{
			// 32-bit Z w/ Stencil
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

			// No Stencil
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;

			// 24-bit Z
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

			// 16-bit Z w/o Stencil
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
			return DXGI_FORMAT_R16_UNORM;

		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}


	DXGI_FORMAT TextureBuffer::getDSVFormat(DXGI_FORMAT format)
	{
		switch (format)
		{
			// 32-bit Z w/ Stencil
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

			// No Stencil
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
			return DXGI_FORMAT_D32_FLOAT;

			// 24-bit Z
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;

			// 16-bit Z w/o Stencil
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
			return DXGI_FORMAT_D16_UNORM;

		default:
			return format;
		}
	}

	/********************/
	/*    DepthBuffer   */
	/********************/

	void DepthBuffer::create(tim::uivec2 res, DXGI_FORMAT format)
	{
		D3D12_RESOURCE_DESC desc = describeTex2D(res, 1, 1, format, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = format;
		clearValue.DepthStencil.Depth = _clearDepth;
		TextureBuffer::create(g_device, desc, clearValue);
		fillDerivedViews(g_device, format);
	}

	void DepthBuffer::fillDepthDSVDescritpor(DXGI_FORMAT format, const Descriptor& descr) const
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Format = getDSVFormat(format);
		if (_resource->GetDesc().SampleDesc.Count == 1)
		{
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = 0;
		}
		else
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;

		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		g_device->CreateDepthStencilView(_resource.Get(), &dsvDesc, descr.cpuHandle());
	}

	
	void DepthBuffer::fillDerivedViews(ID3D12Device* device, DXGI_FORMAT format)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (_DSV_descritpor[i].isNull())
				_DSV_descritpor[i] = g_descriptorsPool[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].alloc(1);
		}

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Format = getDSVFormat(format);
		if (_resource->GetDesc().SampleDesc.Count == 1)
		{
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = 0;
		}
		else
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;

		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		device->CreateDepthStencilView(_resource.Get(), &dsvDesc, _DSV_descritpor[0].cpuHandle());

		dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
		device->CreateDepthStencilView(_resource.Get(), &dsvDesc, _DSV_descritpor[1].cpuHandle());
	}
}