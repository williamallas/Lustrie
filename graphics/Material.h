#pragma once

#include "API.h"
#include <EASTL/shared_ptr.h>
#include "TexturePool.h"

class Material
{
	friend class Graphics;
	friend class dx12::Renderer;

public:
	Material(const Material&) = default;
	Material& operator=(const Material&) = default;

	eastl::shared_ptr<TexturePool> texturePool() const;

private:
	Material() = default;

private:
	eastl::shared_ptr<dx12::RootSignature> _signature;
	eastl::shared_ptr<dx12::PipelineState> _pipeline;
	eastl::shared_ptr<TexturePool> _textures;
};

inline eastl::shared_ptr<TexturePool> Material::texturePool() const { return _textures; }