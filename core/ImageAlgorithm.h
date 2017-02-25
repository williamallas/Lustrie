#pragma once

#include "math/Vector.h"
#include "math/PascaleTriangle.h"
#include "math/Matrix.h"

#include "type.h"

#include <EASTL/string.h>
#include <fstream>

namespace tim
{
    template <class T>
    class ImageAlgorithm
    {
    public:
        ImageAlgorithm() : _data(nullptr), _size(0,0) {}
        ImageAlgorithm(const T* const, uivec2);
        ImageAlgorithm(uivec2);
        ImageAlgorithm(const ImageAlgorithm&);
        ImageAlgorithm(ImageAlgorithm&&);
        ~ImageAlgorithm() { clear(); }

        ImageAlgorithm& operator=(const ImageAlgorithm&);
        ImageAlgorithm& operator=(ImageAlgorithm&&);

        uivec2 size() const { return _size; }
        bool empty() const { return _data == nullptr; }

        T* data() const { return _data; }
        T* detachData();

        template <class F>
        ImageAlgorithm<decltype((*(F*)NULL)(T()))> map(F f) const;

        void set(uint, uint, const T&);

        const T& get(uint x, uint y) const { return safe_get({x,y}); }
        T& get(uint x, uint y) { return safe_get({x,y}); }

        T getLinear(vec2) const;
        T getSmooth(vec2) const;

        const T& clamp_get(int x, int y) const;
        T& clamp_get(int x, int y);

        ImageAlgorithm blured3x3() const;
        template <uint KS> ImageAlgorithm blured() const;

        ImageAlgorithm resized(uivec2) const;
        ImageAlgorithm transformed(const imat2&) const;

        template<class F> void exportBMP(eastl::string, const F&) const; // expect a T -> vec3 function

    private:
        T* _data;
        uivec2 _size;

	private:
        void build(uivec2);
        void clear();

        const T& safe_get(uivec2) const;
        T& safe_get(uivec2);
        const T& get(uivec2) const;
        T& get(uivec2);

        bool check(uivec2 v) const { return v.x() < _size.x() && v.y() < _size.y(); }
    };

    template <class T>
    ImageAlgorithm<T>::ImageAlgorithm(const T* const dat, uivec2 s) : _data(nullptr), _size(0,0)
    {
        if(!dat) return;

        build(s);
        for(uint i=0 ; i<_size.x() ; ++i)
            for(uint j=0 ; j<_size.y() ; ++j)
                get({i,j}) = dat[i*_size.y()+j];
    }

    template <class T>
    ImageAlgorithm<T>::ImageAlgorithm(uivec2 s) : _data(nullptr), _size(0,0)
    {
        build(s);
    }

    template <class T>
    ImageAlgorithm<T>::ImageAlgorithm(const ImageAlgorithm& img) : _data(nullptr), _size(0,0)
    {
        build(img._size);
        for(uint i=0 ; i<_size.x() ; ++i)
            for(uint j=0 ; j<_size.y() ; ++j)
                get({i,j}) = img.get({i,j});
    }

    template <class T>
    ImageAlgorithm<T>::ImageAlgorithm(ImageAlgorithm&& img)
    {
        _data = img._data;
        _size = img._size;
        img._data = nullptr;
        img._size = uivec2(0,0);
    }

    template <class T>
    ImageAlgorithm<T>& ImageAlgorithm<T>::operator=(const ImageAlgorithm& img)
    {
        if(this == &img)
            return *this;

        if(_size != img._size)
            build(img._size);

        for(uint i=0 ; i<_size.x() ; ++i)
            for(uint j=0 ; j<_size.y() ; ++j)
                get({i,j}) = img.get({i,j});
        return *this;
    }

    template <class T>
    ImageAlgorithm<T>& ImageAlgorithm<T>::operator=(ImageAlgorithm&& img)
    {
        clear();
        _data = img._data;
        _size = img._size;
        img._data = nullptr;
        img._size = uivec2(0,0);
        return *this;
    }

    template <class T>
    void ImageAlgorithm<T>::build(uivec2 s)
    {
        clear();
        _size = s;
        _data = new T[s.x()*s.y()];
    }

    template <class T>
    void ImageAlgorithm<T>::clear()
    {
        delete[] _data;
        _data=nullptr;
        _size = uivec2(0,0);
    }

    template <class T>
    const T& ImageAlgorithm<T>::safe_get(uivec2 s) const
    {
        if(check(s)) return _data[s.x()*_size.y()+s.y()];
        else return _data[0];
    }

    template <class T>
    T& ImageAlgorithm<T>::safe_get(uivec2 s)
    {
        if(check(s)) return _data[s.x()*_size.y()+s.y()];
        else return _data[0];
    }

    template <class T>
    const T& ImageAlgorithm<T>::clamp_get(int x, int y) const
    {
        if(empty()) return get(uivec2());
        x = std::max(std::min(x, static_cast<int>(_size.x())-1), 0);
        y = std::max(std::min(y, static_cast<int>(_size.y())-1), 0);
        return get({ static_cast<uint>(x), static_cast<uint>(y) });
    }

    template <class T>
    T& ImageAlgorithm<T>::clamp_get(int x, int y)
    {
        if(empty()) get(uivec2());
        x = std::max(std::min(x, static_cast<int>(_size.x())-1), 0);
        y = std::max(std::min(y, static_cast<int>(_size.y())-1), 0);
        return get({ static_cast<uint>(x), static_cast<uint>(y) });
    }

    template <class T>
    const T& ImageAlgorithm<T>::get(uivec2 s) const
    {
        return _data[s.x()*_size.y()+s.y()];
    }

    template <class T>
    T& ImageAlgorithm<T>::get(uivec2 s)
    {
        return _data[s.x()*_size.y()+s.y()];
    }

    template <class T>
    T ImageAlgorithm<T>::getLinear(vec2 v) const
    {
        const T& nx_ny = clamp_get(int(v.x()), int(v.y()));
        const T& px_ny = clamp_get(int(v.x())+1, int(v.y()));
        const T& nx_py = clamp_get(int(v.x()), int(v.y())+1);
        const T& px_py = clamp_get(int(v.x())+1, int(v.y())+1);

        return interpolate(
                    interpolate(nx_ny, px_ny, v.x()-floorf(v.x())),
                    interpolate(nx_py, px_py, v.x()-floorf(v.x())),
                    v.y() - floorf(v.y()));
    }

    template <class T>
    T ImageAlgorithm<T>::getSmooth(vec2 v) const
    {
        const T& nx_ny = clamp_get(int(v.x()), int(v.y()));
        const T& px_ny = clamp_get(int(v.x())+1, int(v.y()));
        const T& nx_py = clamp_get(int(v.x()), int(v.y())+1);
        const T& px_py = clamp_get(int(v.x())+1, int(v.y())+1);

        v = { v.x()-floorf(v.x()), v.y()-floorf(v.y()) };

        return interpolateCos2(nx_ny, px_ny, nx_py, px_py, v.x(), v.y());
    }

    template <class T>
    void ImageAlgorithm<T>::set(uint x, uint y, const T& dat)
    {
        if(check({x,y}))
            _data[x*_size.y()+y] = dat;
    }

    template <class T>
    T* ImageAlgorithm<T>::detachData()
    {
        T* dat = _data;
        _data = nullptr;
        _size = uivec2(0,0);
        return dat;
    }

    template <class T>
    template <class F>
    ImageAlgorithm<decltype((*(F*)NULL)(T()))> ImageAlgorithm<T>::map(F f) const
    {
        ImageAlgorithm<decltype((*(F*)NULL)(T()))> img(_size);
        for(uint i=0 ; i<_size.x() ; ++i)
            for(uint j=0 ; j<_size.y() ; ++j)
        {
            img.set(i,j, f(get({i,j})));
        }
        return img;
    }

    template <class T>
    ImageAlgorithm<T> ImageAlgorithm<T>::blured3x3() const
    {
        ImageAlgorithm<T> img(_size);
        for(int i=0 ; i<static_cast<int>(_size.x()) ; ++i)
        {
            for(int j=0 ; j<static_cast<int>(_size.y()) ; ++j)
            {
                T c = clamp_get(i-1,j-1)*0.25 + clamp_get(i,j-1)*0.5 + clamp_get(i+1,j-1)*0.25
                    + clamp_get(i-1,j)*0.5 + clamp_get(i,j) + clamp_get(i+1,j)*0.5
                    + clamp_get(i-1,j+1)*0.25 + clamp_get(i,j+1)*0.5 + clamp_get(i+1,j+1)*0.25;

                img.set(i,j, c*0.25);
            }
        }
        return img;
    }

    template <class T>
    template<uint KS>
    ImageAlgorithm<T> ImageAlgorithm<T>::blured() const
    {
        static_assert(KS%2==1, "KS must be odd.");
        static const PascaleTriangle COEF(KS);

        ImageAlgorithm<T> imgH(_size);
        for(int i=0 ; i<static_cast<int>(_size.x()) ; ++i)
            for(int j=0 ; j<static_cast<int>(_size.y()) ; ++j)
        {
            T c = 0;
            for(int k=-(int(KS)-1)/2 ; k<=(int(KS)-1)/2 ; ++k)
                c += clamp_get(i+k,j) * float(float(COEF.getRow(KS-1)[k+(KS-1)/2]) / (1<<(KS-1)));

            imgH.set(i,j, c);
        }

        ImageAlgorithm<T> imgV(_size);
        for(int i=0 ; i<static_cast<int>(_size.x()) ; ++i)
            for(int j=0 ; j<static_cast<int>(_size.y()) ; ++j)
        {
            T c = 0;
            for(int k=-(int(KS)-1)/2 ; k<=(int(KS)-1)/2 ; ++k)
                c += imgH.clamp_get(i,j+k) * float(float(COEF.getRow(KS-1)[k+(KS-1)/2]) / (1<<(KS-1)));

            imgV.set(i,j, c);
        }

        return imgV;
    }

    template <class T>
    ImageAlgorithm<T> ImageAlgorithm<T>::resized(uivec2 s) const
    {
        if(s.x() == 0 || s.y() == 0 || (s == _size) || s.x() > _size.x() || s.y() > _size.y())
            return *this;

        class Foo
        {   public:
            static ImageAlgorithm<T> reduceX(const ImageAlgorithm<T>& img, uint minX)
            {
                uint lp2 = l_power2(img.size().x());
                if(lp2 < minX) lp2 = minX;
                bool times2 = (lp2 == (img.size().x()>>1));

                ImageAlgorithm res({lp2, img.size().y()});
                for(uint i=0 ; i<img.size().y() ; ++i)
                    for(uint j=0 ; j<lp2 ; ++j)
                {
                    if(times2)
                        res.set(j,i, (img.clamp_get(j*2,i)+img.clamp_get(j*2+1,i))/2);
                    else
                    {
                        float ratio = float(img.size().x()) / lp2;
                        float offset = 0.5f*(float(lp2) - (ratio*(lp2-1)));
                        res.set(j,i, img.getLinear(vec2(ratio*j+offset, float(i))));
                    }
                }
                return res;
            }

            static ImageAlgorithm<T> reduceY(const ImageAlgorithm<T>& img, uint minY)
            {
                uint lp2 = l_power2(img.size().y());
                if(lp2 < minY) lp2 = minY;
                bool times2 = (lp2 == (img.size().y()>>1));

                ImageAlgorithm res({lp2, img.size().x()});
                for(uint i=0 ; i<img.size().x() ; ++i)
                    for(uint j=0 ; j<lp2 ; ++j)
                {
                    if(times2)
                        res.set(i,j, (img.clamp_get(i,j*2)+img.clamp_get(i,j*2+1))/2);
                    else
                    {
                        float ratio = float(img.size().y()) / lp2;
                        float offset = 0.5f*(float(lp2) - (ratio*(lp2-1)));
                        res.set(i,j, img.getLinear(vec2(float(i), ratio*j+offset)));
                    }
                }
                return res;
            }
        };


        ImageAlgorithm res;
        while(true)
        {
            if((res.empty() && _size.x() > s.x()) || res.size().x() > s.x())
                res = Foo::reduceX(res.empty() ? *this : res, s.x());

            if((res.empty() && _size.y() > s.y()) || res.size().y() > s.y())
                res = Foo::reduceY(res.empty() ? *this : res, s.y());

            if(!res.empty() && res.size().x() <= s.x() && res.size().y() <= s.y())
                break;
        }
        return res;
    }

    template <class T>
    ImageAlgorithm<T> ImageAlgorithm<T>::transformed(const imat2& m) const
    {
        if(empty()) return *this;

        ivec2 s = ivec2(int(_size.x()), int(_size.y()));
        s = m*s;
        s.x() = abs(s.x());
        s.y() = abs(s.y());

        ImageAlgorithm img(uivec2(uint(s.x()), uint(s.y())));

        for(size_t i=0 ; i<_size.x() ; ++i)
            for(size_t j=0 ; j<_size.y() ; ++j)
        {
            ivec2 coord = m*ivec2(i,j);
            if(coord.x() < 0) coord.x() += s.x();
            if(coord.y() < 0) coord.y() += s.y();

            img.set(coord.x(), coord.y(), get(i,j));
        }

        return img;
    }

    template <class T>
    template<class F>
    void ImageAlgorithm<T>::exportBMP(eastl::string filename, const F& fun) const
    {
        std::ofstream stream(filename.c_str(), std::ios_base::binary);
        if(!stream)
            return;

        byte file[14] = {
            'B','M', // magic
            0,0,0,0, // size in bytes
            0,0, // app data
            0,0, // app data
            40+14,0,0,0 // start of data offset
        };
        byte info[40] = {
            40,0,0,0, // info hd size
            0,0,0,0, // width
            0,0,0,0, // heigth
            1,0, // number color planes
            24,0, // bits per pixel
            0,0,0,0, // compression is none
            0,0,0,0, // image bits size
            0x13,0x0B,0,0, // horz resoluition in pixel / m
            0x13,0x0B,0,0, // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
            0,0,0,0, // #colors in pallete
            0,0,0,0, // #important colors
            };

        int w=_size.x();
        int h=_size.y();

        int padSize  = (4-w%4)%4;
        int sizeData = w*h*3 + h*padSize;
        int sizeAll  = sizeData + sizeof(file) + sizeof(info);

        file[ 2] = (byte)( sizeAll    );
        file[ 3] = (byte)( sizeAll>> 8);
        file[ 4] = (byte)( sizeAll>>16);
        file[ 5] = (byte)( sizeAll>>24);

        info[ 4] = (byte)( w   );
        info[ 5] = (byte)( w>> 8);
        info[ 6] = (byte)( w>>16);
        info[ 7] = (byte)( w>>24);

        info[ 8] = (byte)( h    );
        info[ 9] = (byte)( h>> 8);
        info[10] = (byte)( h>>16);
        info[11] = (byte)( h>>24);

        info[24] = (byte)( sizeData    );
        info[25] = (byte)( sizeData>> 8);
        info[26] = (byte)( sizeData>>16);
        info[27] = (byte)( sizeData>>24);

        stream.write( (char*)file, sizeof(file) );
        stream.write( (char*)info, sizeof(info) );

        byte pad[3] = {0,0,0};

        for (int x=0 ; x<w ; ++x)
        {
            for (int y=0 ; y<h ; ++y)
            {
                bvec3 color = fun(get({uint(x),uint(y)}));
                char* b=reinterpret_cast<char*>(&color);
                stream.write(b+2, 1);
                stream.write(b+1, 1);
                stream.write(b, 1);
            }
            stream.write((char*)pad, padSize);
        }
    }
}
