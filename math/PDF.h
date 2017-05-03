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
    TruncatedGaussianPDF(float mean, float sigma, Vector2<T> range) : _mean(mean), _sigma(sigma), _range(range){}

    TruncatedGaussianPDF(Vector2<T> range) : _range(range)
    {
        _mean =  (_range.x() + _range.y()) * 0.5;
        _sigma = (_range.y() - _range.x()) * 0.25;
    }

    TruncatedGaussianPDF(T range) : TruncatedGaussianPDF(vec2(range)) {}

    T operator()(GENERATOR& gen) const //override
    {
        return T(rtnorm(gen, _range.x(), _range.y(), _mean, _sigma));
    }

    TruncatedGaussianPDF(const TruncatedGaussianPDF&) = default;
    TruncatedGaussianPDF& operator=(const TruncatedGaussianPDF&) = default;

    friend std::ostream& operator<<(std::ostream& os, const TruncatedGaussianPDF<>& pdf);
};

inline std::ostream& operator<<(std::ostream& os, const TruncatedGaussianPDF<>& pdf)
{
    os << "mu=" << (pdf._mean) << " sigma=" << (pdf._sigma) << " range=" << pdf._range.x() << "," << pdf._range.y();
    return os;
}

}

#endif // PDF_TIM

