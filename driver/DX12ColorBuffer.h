#pragma once

#include "DX12Texturebuffer.h"

namespace dx12
{
	// Color buffer resource (for frame buffer)
	/*class ColorBuffer : public TextureBuffer
	{
	public:
		ColorBuffer(tim::vec4 clearColor) : _clearColor(clearColor) {}

		const tim::vec4& clearColor() const;

		void create(tim::uivec2 res, tim::uint numMips = 1, DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT);

	private:
		tim::vec4 _clearColor;

		void fillDerivedViews(ID3D12Device* device, DXGI_FORMAT format);
	};

	const tim::vec4& ColorBuffer::clearColor() const { return _clearColor; }*/
}

