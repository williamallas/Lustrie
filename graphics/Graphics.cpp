#include "Graphics.h"
#include "driver/DX12PipelineState.h"
#include "Planet.h"
#include <core/Chrono.h>

using namespace tim;

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

	material = eastl::make_unique<Material>(createForwardMaterial(true, false));

	return ok;
}


void Graphics::frame(ControlCamera& camera, Planet& planet)
{
	tim::Camera cam;
	cam.clipDist = { 0.1f,1000 };
	cam.dir = camera.position + camera.direction;
	cam.pos = camera.position;
	cam.up = camera.up;
	cam.fov = 70;
	cam.ratio = _screenResolution.x() / float(_screenResolution.y());

	eastl::vector<MeshBuffers*> toDraw;
	planet.cull(cam, toDraw);

	_renderer.render(cam, *material, toDraw);
}


void Graphics::close()
{
	_renderer.close();
}

Material Graphics::createForwardMaterial(bool cullFace, bool wireFrame)
{
	Material mat;
	eastl::vector<dx12::RootSignature::RootParameter> rootParameters(2);

	// const buffer of frame data
	rootParameters[0].setAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_ALL);

	// const buffer of transform matrix
	rootParameters[1].setAsConstantBuffer(1, D3D12_SHADER_VISIBILITY_VERTEX);

	dx12::DX12InputLayout inLayout;
	inLayout.initAsFloatVectors({ {"VERTEX", 3}, {"NORMAL", 3},{ "UV", 2 } });

	dx12::PipelineState::ForwardPipelineParam param;
	param.cullFace = cullFace;
	param.hdr = false;
	param.wireframe = wireFrame;

	mat._signature = eastl::make_shared<dx12::RootSignature>(_renderer.device(), rootParameters);
	mat._pipeline = eastl::make_shared<dx12::PipelineState>(_renderer.device(), mat._signature->rootSignature(), inLayout, dx12::g_shaderSrc, param);

	return mat;
}
