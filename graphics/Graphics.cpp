#include "Graphics.h"
#include "driver/DX12PipelineState.h"
#include "Planet.h"
#include <core/Chrono.h>
#include "geometry\Palette.h"

using namespace tim;

ProxyTexture Graphics::g_dummyTexture;

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

	ImageAlgorithm<float> dummyImg({ 32,32 });
	dummyImg = dummyImg.map([](float) { return 0.f; });
	for (uint i = 0; i < dummyImg.size().x(); ++i) for (uint j = 0; j < dummyImg.size().y(); ++j)
	{
		dummyImg.set(i, j, (i^j) % 2 == 0 ? 0.3f : 1.f);
	}
	
	auto dummyImg2 = dummyImg.resized({ 1024,1024 }).map(tim::Palette());
	g_dummyTexture = ProxyTexture(new dx12::Texture(dummyImg2.size(), DXGI_FORMAT_R8G8B8A8_UNORM, reinterpret_cast<byte*>(dummyImg2.data())));

	return ok;
}


void Graphics::frame(ControlCamera& camera, Material& material, Planet& planet)
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

	_renderer.render(cam, material, toDraw);
}


void Graphics::close()
{
	_renderer.close();
}

Material Graphics::createTexturedForwardMaterial(const char* shaderSrc, int numTextures, bool cullFace, bool wireFrame)
{
	Material mat;
	eastl::vector<dx12::RootSignature::RootParameter> rootParameters(3);

	// const buffer of frame data
	rootParameters[0].setAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_ALL);

	// const buffer of transform matrix
	rootParameters[1].setAsConstantBuffer(1, D3D12_SHADER_VISIBILITY_VERTEX);

	// a texture
	rootParameters[2].setAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, numTextures, D3D12_SHADER_VISIBILITY_PIXEL);
	eastl::vector<D3D12_STATIC_SAMPLER_DESC> samplers(1);
	samplers[0] = dx12::RootSignature::linearWrapSampler(2);
	
	dx12::DX12InputLayout inLayout;
	inLayout.initAsFloatVectors({ {"VERTEX", 3}, {"NORMAL", 3},{ "UV", 2 } });

	dx12::PipelineState::ForwardPipelineParam param;
	param.cullFace = cullFace;
	param.hdr = false;
	param.wireframe = wireFrame;

	mat._signature = eastl::make_shared<dx12::RootSignature>(_renderer.device(), rootParameters, samplers);
	mat._pipeline = eastl::make_shared<dx12::PipelineState>(_renderer.device(), mat._signature->rootSignature(), inLayout, shaderSrc, param);
	mat._textures = eastl::make_shared<TexturePool>(numTextures);

	return mat;
}
