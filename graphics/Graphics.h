#pragma once

#include "API.h"
#include "core/NonCopyable.h"
#include "math/Vector.h"
#include "math/Matrix.h"

#include "Material.h"
#include "MeshBuffers.h"
#include "ControlCamera.h"
#include "TexturePool.h"

class Planet;

class Graphics : NonCopyable
{
public:
	Graphics() = default;
	~Graphics();

	bool init(tim::ivec2, bool, HWND);
	void frame(ControlCamera&, Material&, Planet&);
	void close();

	//Material createForwardMaterial(bool cullFace = true, bool wireFrame = false);
	Material createTexturedForwardMaterial(const char* shader, int numTextures = 16, 
										   bool cullFace = true, bool wireFrame = false);

	static ProxyTexture g_dummyTexture;

private:
	tim::ivec2 _screenResolution;
	dx12::Renderer _renderer;
};

