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

void SampleFunction::addSample(float x)
{
    _samples.push_back(x);
}

void SampleFunction::clear()
{
    _samples.clear();
}

float SampleFunction::operator()(float x) const
{
    if(_samples.empty())
        return 0;

    x = eastl::min(eastl::max(x, 0), 1);
    x = float(_samples.size()-1) * x;

    uint x1 = eastl::min(uint(x), _samples.size()-1);
    uint x2 = eastl::min(uint(x+1), _samples.size()-1);

    x = modf(x, nullptr);

    return tim::interpolateCos(_samples[x1], _samples[x2], x);
}

}

