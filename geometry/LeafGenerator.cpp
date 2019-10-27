#include "LeafGenerator.h"

namespace tim
{

LeafGenerator::LeafGenerator()
{

}

void LeafGenerator::generate(const Parameter& param)
{
    _mesh = UVMesh();

    SampleFunction fun = leafFunction(param.leafType);

    uint resY = param.smoothAlongY ? 3:2;
    eastl::vector<uint> startLine(resY);

    for(uint y=0 ; y<resY ; ++y)
    {
        startLine[y] = _mesh.nbVertices();
        for(uint x=0 ; x<param.resX ; ++x)
        {
            float xcoord = float(x) / (param.resX-1);
            float funEval = 0.5f*fun(xcoord);
			
            if(x == 0 || x == param.resX-1)
            {
                if(y == 1)
                    _mesh.addVertex({Quat::from_axis_angle(vec3(0,1,0), xcoord * param.curvature)({param.size.x() * xcoord,0,0}),
                                     vec2(param.size.x() * xcoord, 0)});
            }
            else if(y == 1)
                _mesh.addVertex({Quat::from_axis_angle(vec3(0,1,0), xcoord * param.curvature)
                                    ({param.size.x() * xcoord, 0, funEval*param.innerCurvature*param.size.x()}),
                                 vec2(param.size.x() * xcoord, 0)});
            else
            {
                float ycoord = (y < 1 ? funEval : -funEval) * param.size.y();
                _mesh.addVertex({Quat::from_axis_angle(vec3(0,1,0), xcoord * param.curvature)({param.size.x() * xcoord, ycoord,
                                                                                               funEval*param.outerCurvature*param.size.x()}),
                                 vec2(param.size.x() * xcoord, ycoord)});
            }
        }
    }

    const int indexLastTopV = param.resX-3;
    for(uint i=0 ; i<param.resX-1 ; ++i)
    {
        if(i == 0)
        {
            _mesh.addFace({{startLine[1], startLine[1]+1, startLine[0], 0}, 3});
            if(param.smoothAlongY)
                _mesh.addFace({{startLine[2], startLine[1]+1, startLine[1], 0}, 3});
        }
        else if(i == param.resX-2)
        {
            _mesh.addFace({{startLine[1]+i, startLine[1]+i+1, startLine[0]+indexLastTopV, 0}, 3});
            if(param.smoothAlongY)
                _mesh.addFace({{startLine[2]+indexLastTopV, startLine[1]+i+1, startLine[1]+i, 0}, 3});
        }
        else
        {
            if(param.triangulate)
            {
                _mesh.addFace({{startLine[1]+i, startLine[1]+i+1, startLine[0]+i, 0}, 3});
                _mesh.addFace({{startLine[0]+i, startLine[0]+i-1, startLine[1]+i, 0}, 3});

                if(param.smoothAlongY)
                {
                    _mesh.addFace({{startLine[2]+i-1, startLine[2]+i, startLine[1]+i+1, 0 }, 3});
                    _mesh.addFace({{startLine[1]+i+1, startLine[1]+i, startLine[2]+i-1, 0 }, 3});
                }

            }
            else
            {
                _mesh.addFace({{startLine[1]+i, startLine[1]+i+1, startLine[0]+i, startLine[0]+i-1}, 4});
                if(param.smoothAlongY)
                    _mesh.addFace({{startLine[2]+i-1, startLine[2]+i, startLine[1]+i+1, startLine[1]+i }, 4});
            }
        }
    }

    _mesh.computeNormals(false);

    if(!param.smoothAlongY)
        _mesh += _mesh.scaled(vec3(1,-1,1)).invertFaces();
}

LeafGenerator::Parameter LeafGenerator::Parameter::gen(int seed, int sizeCategorie)
{
	std::mt19937 gen(seed);
	std::uniform_real_distribution<float> random(-1, 1);

	Parameter param;
	param.smoothAlongY = gen() % 2 == 0;
	param.triangulate = true;
	param.curvature = (random(gen)+1)*0.5f;
	param.innerCurvature = random(gen) * 0.4f;
	param.outerCurvature = random(gen) * 0.4f;
	param.leafType = gen() % 3;
	param.size = { 0.4f + ((random(gen) + 1)*0.5f)*0.6f, 0 };
	param.size.y() = 0.2f + ((random(gen) + 1)*0.5f) * (param.size.x() - 0.2f);

	random = std::uniform_real_distribution<float>(0,1);
	switch (sizeCategorie)
	{
	case 0: 
		param.resX = 4; 
		param.size *= (0.05f + random(gen)*0.2f);
		break;
	case 1: 
		param.resX = 6; 
		param.size *= (0.4f + random(gen)*1.2f);
		break;
	case 2: 
		param.resX = 10; 
		param.size *= (1.5f + random(gen) * 3.f);
		break;
	default : param.resX = 4; break;
	}

	return param;
}

SampleFunction LeafGenerator::leafFunction(int choice)
{
    SampleFunction fun;

    switch(choice)
    {
    case 0:
        for(float x=0 ; x<=1 ; x += 0.1f)
            fun.addSample(4.f * (x*(1 - x)));
		break;

    case 1:
        fun.addSample(0);
        fun.addSample(0.16f*3);
        fun.addSample(0.24f*3);
        fun.addSample(0.28f*3);
        fun.addSample(0.3f*3);
        fun.addSample(0.3f*3);
        fun.addSample(0.26f*3);
        fun.addSample(0.16f*3);
        fun.addSample(0.07f*3);
        fun.addSample(0.055f*3);
        fun.addSample(0);
        break;

    case 2:
    default:
        fun.addSample(0);
        fun.addSample(0.5f);
        fun.addSample(0.75f);
        fun.addSample(0.88f);

        for(int i=0 ; i<10 ; ++i)
            fun.addSample(1);

        fun.addSample(0.92f);
        fun.addSample(0.80f);
        fun.addSample(0.6f);
        fun.addSample(0.3f);
        fun.addSample(0);
        break;
    }

    return fun;
}

}

