#pragma once

#include "DX12Texturebuffer.h"

namespace dx12
{
	// standart texture (for frame buffer)
	class Texture : public TextureBuffer
	{
	public:
		Texture(tim::uivec2 res, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, const byte* data = nullptr);

		void upload(const byte* data, uint64_t* fence = nullptr);

		const Descriptor&  SRV() const;

	private:
		Descriptor _SRV;

		void create(tim::uivec2 res, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, const byte* data = nullptr);
	};

	inline const Descriptor&  Texture::SRV() const { return _SRV; }
}

