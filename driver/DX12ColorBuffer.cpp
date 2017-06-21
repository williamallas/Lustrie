#include "DX12ColorBuffer.h"
#include "DX12.h"
#include <core/Logger.h>

using namespace tim;

namespace dx12
{
	/*void ColorBuffer::create(uivec2 res, uint numMips, DXGI_FORMAT format)
	{
		D3D12_RESOURCE_DESC desc = describeTex2D(res, 1, numMips, format, D3D12_RESOURCE_FLAG_NONE);

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = format;
		clearValue.DepthStencil.Depth = _clearDepth;
		TextureBuffer::create(g_device, desc, clearValue);
		fillDerivedViews(g_device, format);
	}*/
}