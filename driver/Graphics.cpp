#include "Graphics.h"
#include "DX12PipelineState.h"

Graphics::~Graphics()
{
}


bool Graphics::init(tim::ivec2 res, bool fullscreen, HWND handle)
{
	_screenResolution = res;

	InitRendererInfo info;
	info.fullscreen = fullscreen;
	info.vsync = false;
	info.resolution = res;
	info.windowHandle = handle;

	auto ok = _renderer.init(info);
	return ok;
}


void Graphics::frame()
{
	_renderer.render();
}


void Graphics::close()
{
	_renderer.close();
}
