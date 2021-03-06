#include "Planet.h"
#include "math/Frustum.h"

using namespace tim;

const float Planet::NoiseClosure::BASE_SIZE = 60.f;

eastl::unique_ptr<tim::FractalNoise<tim::WorleyNoise<tim::vec3>>> g_fractalWorley3d;

tim::vec3 Planet::computeUp(tim::vec3 pos)
{
	return pos.normalized();
}

Planet::Planet(uint resolution, const Parameter& param, int seedIn) : _parameter(param), _noise(seedIn, param)
{
	g_threadPool.push([=](int thread_id) {
		this->generateLowResGrid(resolution / 8);
		applyNoise(_noise.noiseFun(), 1.f, LOW_RES_PLANET);

		/* Compute normal */
		eastl::vector<BaseMesh*> join(NB_SIDE);
		for (int i = 0; i < NB_SIDE; ++i)
			join[i] = &(this->_planetSideLowRes[i]);
		BaseMesh::computeJoinNormals(join, 1);

		uint64_t fences[NB_SIDE] = { 0 };
		for (int i = 0; i < NB_SIDE; ++i)
			this->_lowResMesh[i] = MeshBuffers::createFromMesh(this->_planetSideLowRes[i], &fences[i]);
		
		for (int i = 0; i < NB_SIDE; ++i)
		{
			dx12::g_commandQueues->waitForFence(fences[i]);
		}
			
		this->_isLowResReady = true;

		this->generateGrid(resolution);

		for (int i = 0; i < NB_SIDE; ++i)
		{
			g_threadPool.push([=](int thread_id) {
				applyNoise(_noise.noiseFun(), 1.f, SIDE_X + i);
				generateSideMeshBuffers(i);
			});
		}
		return 0;
	});
}

vec3 Planet::evalNoise(vec3 v) const
{
	v.normalize();
	v *= (1 + _noise.noiseFun(v*0.5f + 0.5f));
	return v;
}

vec3 Planet::evalNormal(vec3 v, float delta) const
{
	v.normalize();

	vec3 dir = vec3(0, 0, 1);
	if(fabsf(v.dot(dir) - 1) < 0.01)
		dir = vec3(0, 1, 0);

	vec3 axe1 = v.cross(dir);
	vec3 axe2 = v.cross(axe1);

	Mesh smallMesh;
	smallMesh.addVertex(evalNoise(v - axe1*delta));
	smallMesh.addVertex(evalNoise(v));
	smallMesh.addVertex(evalNoise(v - axe2*delta));
	smallMesh.addVertex(evalNoise(v + axe1*delta));
	smallMesh.addVertex(evalNoise(v + axe2*delta));

	smallMesh.addFace({ { 0,1,2,0 }, 3 });
	smallMesh.addFace({ { 1,3,2,0 }, 3 });
	smallMesh.addFace({ { 1,4,3,0 }, 3 });
	smallMesh.addFace({ { 0,4,1,0 }, 3 });

	smallMesh.computeNormals(false);
	return smallMesh.normal(1);
}

float Planet::isFloor(vec3 v) const
{
	v.normalize();
	return _noise.isFloor(v*0.5f + 0.5f);
}

void Planet::cull(const tim::Camera& camera, eastl::vector<ObjectInstance>& visibleBatch)
{
	if (!_isLowResReady)
		return;

	Frustum frust;
	tim::Camera cam = camera;
	frust.buildCameraFrustum(cam, Frustum::NEAR_PLAN);

	mat4 transform = mat4::Translation(_position);

	for (int side = 0; side < NB_SIDE; ++side)
	{
		if (!_isSideReady[side] || (camera.pos - _position).length() > 200 + _parameter.sizePlanet.dot({ 1,1 }))
		{
			if (frust.collide( Sphere(_position, _parameter.sizePlanet.dot({ 1,1 })) ))
				visibleBatch.push_back({ &_lowResMesh[side], transform, MaterialParameter() });
		}
		else
		{
			for (uint i = 0; i < _grid.size(); ++i)
			{
				for (uint j = 0; j < _grid[0].size(); ++j)
				{
					vec3 center = _planetSide[side].position(_grid[i][j][0][0].indexes[0]) +
						       	  _planetSide[side].position(_grid[i][j][0].back().indexes[0]) * 0.5;

					vec3 ray = _planetSide[side].position(_grid[i][j][0][0].indexes[0]) -
							   _planetSide[side].position(_grid[i][j][0].back().indexes[0]);

					if (frust.collide(Sphere(center, max(3*ray.length(), _parameter.sizePlanet.y()*0.5f))))
						visibleBatch.push_back({ &_planetMesh[side][i][j][distanceToLod((center - camera.pos).length())], transform, MaterialParameter() });
				}
			}
		}
	}
}

int Planet::distanceToLod(float dist) const
{
	const float coef = (_parameter.sizePlanet.x() / 50);
	const float baseDist = 80 * coef;
	for (uint i = 0; i < NB_LODS; ++i)
	{
		if (baseDist + i*40*coef > dist)
			return i;
	}

	return NB_LODS -1;
}

void Planet::generateGrid(uint res)
{
    _gridIndex.resize((res+1)*(res+1));
    _gridResolution = res+1;
    UVMesh mesh = UVMesh();

    float d = { 1.f / res };

    uint curIndex = 0;
    for (uint i = 0; i < res+1; ++i)
    {
        for (uint j = 0; j < res+1; ++j)
        {
            vec3 p = vec3(d*i, d*j, 0) - vec3(0.5, 0.5, 0);
            vec2 uv(float(i) / res, float(j) / res);
            mesh.addVertex({p,uv});

			indexGrid(i, j) = curIndex++;
			if (i > 0 && j > 0)
			{
				mesh.addFace({ { indexGrid(i, j - 1), indexGrid(i - 1, j - 1), indexGrid(i,j), 0 }, 3 });
				mesh.addFace({ { indexGrid(i - 1, j), indexGrid(i, j), indexGrid(i - 1, j - 1), 0 }, 3 });
			}
        }
    }

    _planetSide[SIDE_X] = mesh.rotated(mat3(mat3::RotationY(toRad(90)) * mat3::RotationZ(toRad(0)))).translated(vec3(0.5,0,0));
    _planetSide[SIDE_NX] = mesh.rotated(mat3(mat3::RotationY(toRad(-90)) * mat3::RotationZ(toRad(180)))).translated(vec3(-0.5,0,0));

    _planetSide[SIDE_Y] = mesh.rotated(mat3(mat3::RotationX(toRad(-90)) * mat3::RotationZ(toRad(90)))).translated(vec3(0,0.5,0));
    _planetSide[SIDE_NY] = mesh.rotated(mat3(mat3::RotationX(toRad(90)) * mat3::RotationZ(toRad(-90)))).translated(vec3(0,-0.5,0));

    _planetSide[SIDE_Z] = mesh.translated(vec3(0,0,0.5));
    _planetSide[SIDE_NZ] = mesh.rotated(mat3::RotationX(toRad(180))).translated(vec3(0,0,-0.5));

    generateBatchIndex(res, true);
}

void Planet::generateBatchIndex(tim::uint res, bool triangulate)
{
    for(auto& l : _grid)
        for(auto& r : l)
            for(auto& lod : r)
                lod.clear();

    for(uint i=0 ; i<_grid.size() ; ++i)
    {
        for(uint j=0 ; j<_grid.size() ; ++j)
        {
            for(uint lod = 0 ; lod < _grid[i][j].size() ; ++lod)
            {
                uint batchRes = res / _grid.size();
                uint stride = 1 << lod;

                for (uint x = 0; x < batchRes+1; x += stride)
                {
                    for (uint y = 0; y < batchRes+1; y += stride)
                    {
                        uint ox = i*batchRes;
                        uint oy = j*batchRes;
                        uint xx=x+ox, yy=y+oy;

                        if (x > 0 && y > 0)
                        {
                            if (triangulate)
                            {
                                _grid[i][j][lod].push_back({ { indexGrid(xx,yy-stride), indexGrid(xx - stride,yy - stride), indexGrid(xx,yy), 0 }, 3 });
                                _grid[i][j][lod].push_back({ { indexGrid(xx-stride,yy), indexGrid(xx,yy), indexGrid(xx-stride,yy-stride), 0 }, 3 });
                            }
                            else
                                _grid[i][j][lod].push_back({ { indexGrid(xx-stride,yy-stride), indexGrid(xx,yy-stride), indexGrid(xx,yy), indexGrid(xx-stride,yy) }, 4 });
                        }
                    }
                }
            }
        }
    }
}

void Planet::generateLowResGrid(tim::uint res)
{
	UVMesh plan = UVMesh::generateGrid(vec2(1, 1), { res, res }, ImageAlgorithm<float>(), 0, true);
	plan.invertFaces();

	_planetSideLowRes[SIDE_Z] = plan.translated(vec3(0, 0, 0.5));
	_planetSideLowRes[SIDE_NZ] = plan.translated(vec3(0, 0, 0.5)).scaled(vec3(1, 1, -1)).invertFaces();

	_planetSideLowRes[SIDE_Y] = plan.rotated(mat3(mat3::RotationX(toRad(-90)) * mat3::RotationZ(toRad(90)))).translated(vec3(0, 0.5, 0));
	_planetSideLowRes[SIDE_NY] = plan.rotated(mat3(mat3::RotationX(toRad(90)) * mat3::RotationZ(toRad(-90)))).translated(vec3(0, -0.5, 0));

	_planetSideLowRes[SIDE_X] = plan.rotated(mat3(mat3::RotationY(toRad(90)) * mat3::RotationZ(toRad(0)))).translated(vec3(0.5, 0, 0));
	_planetSideLowRes[SIDE_NX] = plan.rotated(mat3(mat3::RotationY(toRad(-90)) * mat3::RotationZ(toRad(180)))).translated(vec3(-0.5, 0, 0));
}

tim::UVMesh Planet::generateMesh(vec3 pos) const
{
    UVMesh result;
    for(int side=0 ; side<NB_SIDE ; ++side)
    {
        UVMesh sideMesh = _planetSide[side];
        for(uint i=0 ; i<_grid.size() ; ++i)
        {
            for(uint j=0 ; j<_grid.size() ; ++j)
            {
                //int index = int( (_grid[i][j].sphere.center() - vec3(0,0,1)).length() * 4 + 0.5);
                for(auto f : _grid[i][j][0])
                    sideMesh.addFace(f);
            }
        }
        result += sideMesh;
    }
    result.computeNormals(false);

    return result;
}

void Planet::generateSideMeshBuffers(int side)
{
	uint indexOffsetlod[NB_LODS] = { 0 };
	GridType < eastl::shared_ptr<dx12::GpuBuffer> > indexBufferAllLods;

	uint64_t fences[2] = { 0 };

	_planetSide[side].computeNormals(false, 16);
	eastl::shared_ptr<dx12::GpuBuffer> vb = MeshBuffers::createVertexBufferFromMesh(_planetSide[side], &fences[0]);

	uint nbIndexConcat = 0;
	for (uint i = 0; i < NB_LODS; ++i)
	{
		indexOffsetlod[i] = nbIndexConcat;
		nbIndexConcat += _grid[0][0][i].size() * 3;
	}

	auto& commandContext = dx12::CommandContext::AllocContext(dx12::CommandQueue::COPY);

	for (uint i = 0; i < _grid.size(); ++i)
	{
		for (uint j = 0; j < _grid[0].size(); ++j)
		{
			dx12::GpuBuffer* ib = new dx12::GpuBuffer(nbIndexConcat, sizeof(tim::uint));
			for (uint lod = 0; lod < NB_LODS; lod++)
			{
				BaseMesh tmp;
				for (auto f : _grid[i][j][lod])
					tmp.addFace(f);

				commandContext.initBuffer(*ib, tmp.indexData().data(), tmp.nbFaces() * 3 * sizeof(uint), indexOffsetlod[lod] * sizeof(uint));
			}

			indexBufferAllLods[i][j] = eastl::shared_ptr<dx12::GpuBuffer>(ib);
			for (uint lod = 0; lod < NB_LODS; ++lod)
				_planetMesh[side][i][j][lod] = MeshBuffers(vb, indexBufferAllLods[i][j], indexOffsetlod[lod], _grid[i][j][lod].size() * 3);
		}
	}

	commandContext.finish(true);
	_isSideReady[side] = true;
	//dx12::g_commandQueues->waitForFence(fences[0]);
}

Planet::Parameter Planet::Parameter::generate(int seed)
{
	std::mt19937 randEngine(seed);
	std::uniform_real_distribution<float> random;
	
	Parameter param;
	vec3 distri(powf(random(randEngine), 3.f), powf(random(randEngine), 3.f), powf(random(randEngine), 3.f));
	distri.normalize();
	if (distri.z() < 0.5)
		distri.z() = 0;
	distri.normalize();

	param.sizePlanet.x() = 40 + random(randEngine) * 100;
	float scaleMountain = random(randEngine);
	param.sizePlanet.y() = param.sizePlanet.x() * scaleMountain * 1.5f;
	
	float freqFactor = 1;
	if (scaleMountain > 0.5)
		freqFactor = 1.2f - scaleMountain;

	param.largeRidgeZScale = distri[0];
	param.largeSimplexZScale = distri[1];
	param.largeWorleyZScale = distri[2];
	param.floorHeight = 0.2f + (param.largeRidgeZScale + param.largeSimplexZScale + param.largeWorleyZScale) * random(randEngine) * 0.5f;

	param.largeRidgeCoef = freqFactor * (param.sizePlanet.x() / 50) * (0.75f + random(randEngine) * 0.5f);
	param.largeSimplexCoef = freqFactor * (param.sizePlanet.x() / 50) * (0.75f + random(randEngine) * 0.5f);
	param.largeWorleyCoef = freqFactor * (param.sizePlanet.x() / 50) * (0.75f + random(randEngine) * 0.5f);

	param.invertWorley = randEngine() % 2 == 0;
	param.largeExponent = 1.f;// +powf(random(randEngine), 2.f);
	param.simplexDetailZScale = 0.005f + random(randEngine) * 0.02f;
	param.simplexDetailCoef = 20 + random(randEngine) * 20.f;

	return param;
}