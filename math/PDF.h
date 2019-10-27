#ifndef PDF_TIM
#define PDF_TIM

#include "Vector.h"
#include <iostream>

namespace tim
{

/*template<typename T = float, class GENERATOR = std::mt19937>
class PDF
{
public: virtual T operator()(GENERATOR&) const = 0;
        virtual ~PDF() {}
};*/

template<typename T = float, class GENERATOR = std::mt19937>
class UniformPDF //: public PDF<T,GENERATOR>
{
public:
    UniformPDF(Vector2<T> range) : _distri(range.x(), range.y()) {}

    T operator()(GENERATOR& gen) const //override
    {
        return _distri(gen);
    }

private:
    mutable std::uniform_real_distribution<T> _distri;
};

template<typename T = float, class GENERATOR = std::mt19937>
class TruncatedGaussianPDF //: public PDF<T,GENERATOR>
{
    T _mean = 0.5;
    T _sigma = 1;
    Vector2<T> _range = {0,1};

public:
	TruncatedGaussianPDF() = default;
    TruncatedGaussianPDF(float mean, float sigma, Vector2<T> range) : _mean(mean), _sigma(sigma), _range(range){}

    TruncatedGaussianPDF(Vector2<T> range) : _range(range)
    {
        _mean =  (_range.x() + _range.y()) * 0.5f;
        _sigma = (_range.y() - _range.x());
    }

    TruncatedGaussianPDF(T range) : TruncatedGaussianPDF(vec2(range)) {}

    T operator()(GENERATOR& gen) const //override
    {
        return T(rtnorm(gen, _range.x(), _range.y(), _mean, _sigma));
    }

    TruncatedGaussianPDF(const TruncatedGaussianPDF&) = default;
    TruncatedGaussianPDF& operator=(const TruncatedGaussianPDF&) = default;

    TruncatedGaussianPDF& operator*=(T x)
    {
        _mean *= x;
        _sigma *= x;
        _range *= x;
        return *this;
    }

    TruncatedGaussianPDF operator+(T x) const
    {
        TruncatedGaussianPDF pdf = *this;
        return pdf += x;
    }

	TruncatedGaussianPDF& operator+=(T x)
	{
		_mean += x;
		_range += x;
		return *this;
	}

	TruncatedGaussianPDF operator*(T x) const
	{
		TruncatedGaussianPDF pdf = *this;
		return pdf *= x;
	}

	TruncatedGaussianPDF& makePositive()
	{
		_mean = std::max(0.f, _mean);
		_range[0] = std::max(0.f, _range[0]);
		_range[1] = std::max(0.05f, _range[1]);
		return *this;
	}

	TruncatedGaussianPDF& translate_scale(float tr, float sc)
	{
		float med = (_range[0] + _range[1]) * 0.5f;
		return *this = ((*this + (-med))*sc) + med + tr;
	}

	static TruncatedGaussianPDF interpolate(const TruncatedGaussianPDF& pdf1, const TruncatedGaussianPDF& pdf2, float coef)
	{
		TruncatedGaussianPDF res;
		res._mean = tim::interpolate(pdf1._mean, pdf2._mean, coef);
		res._sigma = tim::interpolate(pdf1._sigma, pdf2._sigma, coef);
		res._range = tim::interpolate(pdf1._range, pdf2._range, coef);
		return res;
	}

	std::ostream& print(std::ostream& os) const
	{
		os << "mu=" << (_mean) << " sigma=" << (_sigma) << " range=" << _range.x() << "," << _range.y();
		return os;
	}
};





}

#endif // PDF_TIM

