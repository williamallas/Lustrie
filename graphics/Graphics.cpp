#include "Graphics.h"
#include "driver/DX12PipelineState.h"
#include "PlanetSystem.h"
#include <core/Chrono.h>
#include "geometry\Palette.h"
#include "TextureGenerator.h"

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

	// create standart root rignature
	eastl::vector<dx12::RootSignature::RootParameter> rootParameters(2);
	int slot = 0;
	rootParameters[slot++].setAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_ALL);
	//rootParameters[slot++].setAsConstantBuffer(1, D3D12_SHADER_VISIBILITY_ALL);

	rootParameters[slot++].setAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, SIZE_TEXTURE_POOL, D3D12_SHADER_VISIBILITY_PIXEL);
	eastl::vector<D3D12_STATIC_SAMPLER_DESC> samplers(1);
	samplers[0] = dx12::RootSignature::trilinearWrapSampler(2);

	_standartRootSignature = eastl::make_shared<dx12::RootSignature>(_renderer.device(), rootParameters, samplers);

	// create default texture
	ImageAlgorithm<float> dummyImg({ 32,32 });
	dummyImg = dummyImg.map([](float) { return 0.f; });
	for (uint i = 0; i < dummyImg.size().x(); ++i) for (uint j = 0; j < dummyImg.size().y(); ++j)
	{
		dummyImg.set(i, j, (i^j) % 2 == 0 ? 0.3f : 1.f);
	}
	
	auto dummyImg2 = dummyImg.resized({ 1024,1024 }).map(tim::Palette());
	g_dummyTexture = createTextureWithMips(dummyImg2);

	return ok;
}


void Graphics::frame(ControlCamera& camera, PlanetSystem& planet)
{
	tim::Camera cam;
	cam.clipDist = { 0.1f,1000 };
	cam.dir = camera.position + camera.direction;
	cam.pos = camera.position;
	cam.up = camera.up;
	cam.fov = 70;
	cam.ratio = _screenResolution.x() / float(_screenResolution.y());

	eastl::vector<ObjectInstance> toDraw;
	planet.planet->cull(cam, toDraw);

	eastl::vector<ObjectInstance> toDrawGrass;
	for(const auto& grass : planet.grassOnPlanet)
		grass->cull(cam, toDrawGrass);

	eastl::vector<ObjectInstance> toDrawPlants, toDrawLeafs;
	eastl::vector<mat4> plantsModelMatrix, leafModelMatrix;
	if(planet.plants)
		planet.plants->cull(cam, toDrawPlants, toDrawLeafs);

	_renderer.beginRender(cam, _timeElapsed);

	if(!toDraw.empty())
		_renderer.render(*planet.planetMaterial, toDraw);

	if(!toDrawGrass.empty())
		_renderer.render(*planet.grassMaterial[0], toDrawGrass);

	if (!toDrawLeafs.empty())
		_renderer.render(*planet.leafMaterial, toDrawLeafs);

	if (!toDrawPlants.empty())
		_renderer.render(*planet.plantMaterial, toDrawPlants);
		
	_renderer.endRender();
}


void Graphics::close()
{
	_renderer.close();
}

Material Graphics::createTexturedForwardMaterial(const char* shaderSrc, const eastl::shared_ptr<TexturePool>& pool, bool cullFace, bool wireFrame)
{
	Material mat;

	dx12::DX12InputLayout inLayout;
	inLayout.initAsFloatVectors({ {"VERTEX", 3}, {"NORMAL", 3},{ "UV", 2 } });
	inLayout.addMat4PerInstanceElement({ "MODEL_MATRIX" });
	inLayout.addPerInstanceMaterial();

	dx12::PipelineState::ForwardPipelineParam param;
	param.cullFace = cullFace;
	param.hdr = false;
	param.wireframe = wireFrame;

	mat._signature = _standartRootSignature;
	mat._pipeline = eastl::make_shared<dx12::PipelineState>(_renderer.device(), mat._signature->rootSignature(), inLayout, shaderSrc, param);
	mat._textures = pool;

	return mat;
}

Material Graphics::createPointToTriangleGSForwardMaterial(const char* shaderSrc, const eastl::shared_ptr<TexturePool>& pool, bool cullFace, bool wireFrame)
{
	Material mat;
	
	dx12::DX12InputLayout inLayout;
	inLayout.initAsFloatVectors({ { "VERTEX", 3 },{ "NORMAL", 3 } });
	inLayout.addMat4PerInstanceElement({ "MODEL_MATRIX" });
	inLayout.addPerInstanceMaterial();

	dx12::PipelineState::ForwardPipelineParam param;
	param.cullFace = cullFace;
	param.hdr = false;
	param.wireframe = wireFrame;
	param.primitive = dx12::PipelineState::Point;

	mat._signature = _standartRootSignature;
	mat._pipeline = eastl::make_shared<dx12::PipelineState>(_renderer.device(), mat._signature->rootSignature(), inLayout, shaderSrc, param, true);
	mat._textures = pool;

	return mat;
}

ProxyTexture Graphics::createTextureWithMips(const tim::ImageAlgorithm<tim::bvec4>& img)
{
	auto mips = TextureGenerator::generateMips(img);
	auto t = new dx12::Texture(img.size(), mips.size() + 1, DXGI_FORMAT_R8G8B8A8_UNORM);
	eastl::vector<const byte*> mips_data(mips.size() + 1);
	mips_data[0] = reinterpret_cast<byte*>(img.data());

	for (size_t i = 0; i < mips.size(); ++i)
		mips_data[i + 1] = reinterpret_cast<byte*>(mips[i].data());

	t->upload(mips_data);

	return ProxyTexture(t);
}
