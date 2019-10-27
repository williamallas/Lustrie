#include "SampleFunction.h"
#include "core/type.h"
#include "math/math.h"

namespace tim
{


SampleFunction::SampleFunction()
{

}

SampleFunction::SampleFunction(const eastl::vector<float>& samples) : _samples(samples)
{

}

SampleFunction::SampleFunction(std::initializer_list<float> args) : _samples(args.begin(), args.end())
{

}

SampleFunction& SampleFunction::addSample(float x)
{
    _samples.push_back(x);
    return *this;
}

void SampleFunction::clear()
{
    _samples.clear();
}

float SampleFunction::operator()(float x) const
{
    if(_samples.empty())
        return 0;

    x = eastl::min(eastl::max(x, 0.f), 1.f);
    x = float(_samples.size()-1) * x;

    uint x1 = eastl::min(uint(x), _samples.size()-1);
    uint x2 = eastl::min(uint(x+1), _samples.size()-1);

    x = fmodf(x, 1);

    return tim::interpolate(_samples[x1], _samples[x2], x);
}

SampleFunction SampleFunction::interpolate(const SampleFunction& f1, const SampleFunction& f2, float coef)
{	
	uint nbPts = eastl::max(f1._samples.size(), f2._samples.size());

	if (nbPts == 0)
		return SampleFunction();
	else if (nbPts == 1)
		return SampleFunction({ tim::interpolate(f1(0.5f), f2(0.5f), coef) });
	
	SampleFunction result;
	for (uint i = 0; i < nbPts; ++i)
		result.addSample(tim::interpolate(f1(float(i) / (nbPts - 1)), f2(float(i) / (nbPts - 1)), coef));

	return result;
}

}

