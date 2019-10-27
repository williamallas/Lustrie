#pragma once

#include "DX12Texturebuffer.h"

namespace dx12
{
	// standart texture (for frame buffer)
	class Texture : public TextureBuffer
	{
	public:
		explicit Texture(tim::uivec2 res, tim::uint numMips = 1, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, const byte* data = nullptr);

		void upload(const byte* data, uint64_t* fence = nullptr);
		void upload(eastl::vector<const byte*> mips, uint64_t* fence = nullptr);

		const Descriptor&  SRV() const;

	private:
		Descriptor _SRV;

		void create(tim::uivec2 res, tim::uint numMips = 1, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, const byte* data = nullptr);
	};

	inline const Descriptor&  Texture::SRV() const { return _SRV; }
}

