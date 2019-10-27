#pragma once

#include "API.h"
#include "core/NonCopyable.h"
#include "math/Vector.h"
#include "math/Matrix.h"

#include "Material.h"
#include "MeshBuffers.h"
#include "ControlCamera.h"
#include "TexturePool.h"

class PlanetSystem;

class Graphics : NonCopyable
{
public:
	static constexpr int SIZE_TEXTURE_POOL = 16;

	Graphics() = default;
	~Graphics();

	void setTimeElapsed(float time) { _timeElapsed = time; }

	bool init(tim::ivec2, bool, HWND);
	void frame(ControlCamera&, PlanetSystem&);
	void close();

	//Material createForwardMaterial(bool cullFace = true, bool wireFrame = false);
	Material createTexturedForwardMaterial(const char* shader, const eastl::shared_ptr<TexturePool>& pool = eastl::make_shared<TexturePool>(16),
										   bool cullFace = true, bool wireFrame = false);

	Material createPointToTriangleGSForwardMaterial(const char* shader, const eastl::shared_ptr<TexturePool>& pool = eastl::make_shared<TexturePool>(16),
										   bool cullFace = true, bool wireFrame = false);

	static ProxyTexture g_dummyTexture;

	static ProxyTexture createTextureWithMips(const tim::ImageAlgorithm<tim::bvec4>&);

private:
	tim::ivec2 _screenResolution;
	dx12::Renderer _renderer;
	float _timeElapsed;

	eastl::shared_ptr<dx12::RootSignature> _standartRootSignature;
};

