#pragma once

#include "core/type.h"
#include "core/ImageAlgorithm.h"
#include <random>
#include <EASTL/sort.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/deque.h>
#include <EASTL/algorithm.h>

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

namespace tim
{
    template<class T>
    class WorleyNoise
    {
    public:
        using Point = T;
        WorleyNoise(uint nbPoints, int nth = 1, int seed = 42);

        WorleyNoise(const WorleyNoise&) = delete;
        WorleyNoise& operator=(const WorleyNoise&) = delete;

        WorleyNoise(WorleyNoise&&) = default;
        WorleyNoise& operator=(WorleyNoise&&) = default;

        float noise(T x) const;

    private:
        int _nth;
        eastl::vector<T> _points;
        float _sqrtNbPoints;

        struct Node
        {
            Node *left, *right;
            eastl::vector<uint> container;
        };

        eastl::deque<Node, EASTLAllocatorType, 256> _nodePool;
        Node* _root = nullptr;

        void optimiseSpace(Node*, Point, float, uint depth, uint maxDepth);
        float search(Point, Node*, Point, float, uint depth) const;
    };

    template<class T>
    struct WorleyNoiseInstancer
    {
        uint _nbPoints, _layerCoef;
        int _nth, _seed;

        WorleyNoiseInstancer(uint nbPoints, uint layerCoef = 3, uint nth = 1, int seed = 42) : _nbPoints(nbPoints), _layerCoef(layerCoef), _nth(nth), _seed(seed) {}

        T operator()(uint layer) const
        {
            return T(_nbPoints * uipow(_layerCoef, layer), _nth, _seed+layer);
        }
    };

    /********************/
    /*** Implentation ***/
    /********************/

    /** Worley Noise **/

    template<class T> WorleyNoise<T>::WorleyNoise(uint nbPoints, int nth, int seed) : _nth(nth), _nodePool{}, _root{&_nodePool.push_back()}
    {
        std::mt19937 randEngine(seed);
        std::uniform_real_distribution<float> random(0,1);

        for(uint i=0 ; i<nbPoints ; ++i)
        {
            T v;
            for(uint j=0 ; j<T::Length ; ++j)
                v[j] = random(randEngine);
            _points.push_back(v);
        }

        int maxDepth = int(log2f(float(nbPoints)) / Point::Length + 0.5f);
        maxDepth = min(8, max(0, maxDepth));
        //std::cout << "maxdepth:" << maxDepth << " nbPts:" << nbPoints<<std::endl;
        optimiseSpace(_root, Point(0.5f), 0.5f, 0, maxDepth);

        _sqrtNbPoints = powf(float(nbPoints), 1.f/float(Point::Length));
    }

    template<class T> float WorleyNoise<T>::noise(T x) const
    {
        x.apply([](float val) { return fmodf(fabsf(val), 1.f); });

        float result=0;
        if(_root == nullptr)
        {
            int nth = _nth>0 ? _nth:1;
            eastl::vector<float> dists(_points.size());
            for(size_t i=0 ; i<_points.size() ; ++i)
                dists[i] = (_points[i] - x).length2();

            eastl::partial_sort(dists.begin(), dists.begin()+nth, dists.end());

            result = sqrtf(dists[nth-1]);
        }
        else
            result = search(x, _root, Point(0.5f), 0.5f, 0);

        return min(result*_sqrtNbPoints, 1.f);
    }

    namespace
    {
        template<class T> bool isIn(T p, T center, float size)
        {
            p -= center;
            for(uint i=0 ; i<T::Length ; ++i)
            {
                if(fabsf(p[i]) > size)
                    return false;
            }
            return true;
        }
    }

    template<class T> void WorleyNoise<T>::optimiseSpace(Node* node, Point center, float size, uint depth, uint maxDepth)
    {
        if(depth % Point::Length==0)
        {
            for(uint i=0 ; i<_points.size() ; ++i)
            {
                if(isIn(_points[i], center, size*2))
                    node->container.push_back(i);
            }
        }

        if(depth/Point::Length == maxDepth || (node->container.empty() && depth % Point::Length==0))
        {
            node->left = nullptr;
            node->right = nullptr;
        }
        else
        {
            int state = depth % Point::Length;
            node->left = &_nodePool.push_back();
            node->right = &_nodePool.push_back();

            Point lcenter = center, rcenter = center;
            lcenter[state] -= size*0.5f;
            rcenter[state] += size*0.5f;

            if(depth % Point::Length == Point::Length-1)
                size *= 0.5f;

            optimiseSpace(node->left, lcenter, size, depth+1, maxDepth);
            optimiseSpace(node->right, rcenter, size, depth+1, maxDepth);
        }
    }

    template<class T> float WorleyNoise<T>::search(Point p, Node* node, Point center, float size, uint depth) const
    {
        int state = depth % Point::Length;
        Node* next_node = nullptr;
        if(p[state] < center[state])
        {
            center[state] -= size*0.5f;
            next_node = node->left;
        }
        else
        {
            center[state] += size*0.5f;
            next_node = node->right;
        }

        if(depth % Point::Length == Point::Length-1)
            size *= 0.5f;

        if(depth%Point::Length == 0)
        {
            float d = -1;
            if(next_node)
                d = search(p, next_node, center, size, depth+1);

            if(d < 0)
            {
                int nth = _nth>0 ? _nth:1;
                eastl::vector<float> dists;

                for(size_t i=0 ; i<node->container.size() ; ++i)
                {
					uint gg = node->container[i];
                    float dist = (_points[ node->container[i] ] - p).length2();

                    if(dist < size*size)
                        dists.push_back(dist);
                }

                if(int(dists.size()) < nth)
                    return -1;

                eastl::partial_sort(dists.begin(), dists.begin()+nth, dists.end());

                return sqrtf(dists[nth-1]);
            }
            else return d;
        }
        else
            return search(p, next_node, center, size, depth+1);
    }

}
