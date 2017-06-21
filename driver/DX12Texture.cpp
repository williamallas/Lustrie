#include "DX12Texture.h"
#include "DX12.h"
#include <core/Logger.h>

#include "DX12CommandList.h"

using namespace tim;

namespace dx12
{
	Texture::Texture(uivec2 res, DXGI_FORMAT format, const byte* data)
	{
		create(res, format, data);
	}

	namespace
	{
		size_t bytesPerPixel(DXGI_FORMAT f) { return TextureBuffer::bitsPerPixel(f) / 8; }
	};

	void Texture::create(uivec2 res, DXGI_FORMAT format, const byte* data)
	{
		D3D12_RESOURCE_DESC desc = describeTex2D(res, 1, 1, format, D3D12_RESOURCE_FLAG_NONE);
		desc.Format = format;
		TextureBuffer::create(g_device, desc, nullptr);

		if (data)
		{
			upload(data);
		}
		
		_SRV = g_descriptorsPool[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].alloc(1);
		g_device->CreateShaderResourceView(_resource.Get(), nullptr, _SRV.cpuHandle());
	}

	void Texture::upload(const byte* data, uint64_t* fence)
	{
		CommandContext& commandlist = CommandContext::AllocContext(CommandQueue::DIRECT);
		
		eastl::vector<D3D12_SUBRESOURCE_DATA> res(1);
		res[0].pData = data;
		res[0].RowPitch = _size.x() * bytesPerPixel(_format);
		res[0].SlicePitch = res[0].RowPitch * _size.y();
		commandlist.initTexture(*this, res);

		if(fence)
			*fence = commandlist.finish(false);
		else
			commandlist.finish(true);
	}
}