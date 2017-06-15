#pragma once

#include "API.h"
#include "core/NonCopyable.h"
#include "math/Vector.h"
#include "math/Matrix.h"

#include "Material.h"
#include "MeshBuffers.h"
#include "ControlCamera.h"

class Planet;

class Graphics : NonCopyable
{
public:
	Graphics() = default;
	~Graphics();

	bool init(tim::ivec2, bool, HWND);
	void frame(ControlCamera&, Planet&);
	void close();

	Material createForwardMaterial(bool cullFace = true, bool wireFrame = false);

private:
	tim::ivec2 _screenResolution;
	dx12::Renderer _renderer;

	eastl::unique_ptr<Material> material;
};

