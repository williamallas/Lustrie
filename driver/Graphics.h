#pragma once

#include "DX12Renderer.h"
#include "core/NonCopyable.h"
#include "math/Vector.h"

namespace api = dx12;

class Graphics : NonCopyable
{
public:
	Graphics() = default;
	~Graphics();

	bool init(tim::ivec2, bool, HWND);
	void frame();
	void close();

private:
	tim::ivec2 _screenResolution;
	api::Renderer _renderer;
};

