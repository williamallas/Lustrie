

#include "Curve.h"

namespace tim
{
    Mesh Curve::convertToWireMesh() const
    {
        Mesh mesh;
        if(_points.empty())
            return mesh;


        mesh._vertices.resize(_points.size());
        for(size_t i=0 ; i<_points.size() ; ++i)
            mesh._vertices[i] = _points[i].to<3>();

        for(size_t i=0 ; i<_points.size()-1 ; ++i)
            mesh.addFace(Mesh::Face({{i,i+1,0,0}, 2}));

        if(_closed)
            mesh.addFace(Mesh::Face({{_points.size()-1,0,0,0}, 2}));

        return mesh;
    }

    Mesh Curve::convertToMesh(uint resolution, bool mergeLast, bool triangle) const
    {
        return convertToMesh([this](float,float,int index){ return _points[index].w(); }, resolution, mergeLast, triangle);
    }

    UVMesh Curve::convertToUVMesh(uint resolution, bool mergeLast, bool triangle) const
    {
        return convertToUVMesh([this](float,float,int index){ return _points[index].w(); }, resolution, mergeLast, triangle);
    }

    void Curve::tesselateCylindre(BaseMesh& mesh, const eastl::vector<uint>& bottom, const eastl::vector<uint>& top, uint resolution, bool triangulate)
    {
        for(uint i=0 ; i<resolution ; ++i)
        {
            if(triangulate)
            {
                mesh.addFace({{bottom[i%resolution], top[i%resolution], bottom[(i+1)%resolution], 0}, 3});
                mesh.addFace({{top[i%resolution], top[(i+1)%resolution], bottom[(i+1)%resolution], 0}, 3});
            }
            else
            {
                mesh.addFace({{bottom[i%resolution], top[i%resolution],
                           top[(i+1)%resolution], bottom[(i+1)%resolution]}, 4});
            }
        }
    }

    void Curve::tesselateCone(BaseMesh& mesh, const eastl::vector<uint>& bottom, uint top, uint resolution)
    {
        for(uint i=0 ; i<resolution ; ++i)
            mesh.addFace({{bottom[i%resolution], top, bottom[(i+1)%resolution], 0}, 3});
    }

	vec3 Curve::computeDirection(uint index) const
	{
		if (_points.size() <= 1 || index >= _points.size())
			return vec3();

		if (index == 0)
            return (_points[index + 1].to<3>() - _points[index].to<3>()).normalized();
		else if(index == _points.size()-1)
            return (_points[index].to<3>() - _points[index - 1].to<3>()).normalized();
		else
            return (_points[index + 1].to<3>() - _points[index - 1].to<3>()).normalized();
	}

    void Curve::aligneCircle(const eastl::vector<eastl::pair<vec3,vec2>>& base, eastl::vector<eastl::pair<vec3,vec2>>& top)
    {
        struct detail
        {
            // the standard formula to compute the variance of a vec3 array
            static float variance(const eastl::vector<vec3>& vec)
            {
                vec3 mean = 0;
                for(auto v : vec) mean += v;
                mean /= vec.size();

                float result = 0;
                for(auto v:  vec)
                    result += (v-mean).dot(v-mean);
                return result / vec.size();
            }
        };

        if(base.size() != top.size())
            return;

        // compute the variance score for each configuration
        eastl::vector<float> var_score(base.size());
        for(int i=0 ; i<int(base.size()) ; ++i)
        {
            eastl::vector<vec3> acc;
            for(int j=0 ; j<int(base.size()) ; ++j)
               acc.push_back( (base[j].first - top[(i+j)%base.size()].first) );
            var_score[i] = detail::variance(acc);
        }

        // select the min variance (such that the edges are aligned)
        int bestIndex=0;
        for(int i=0 ; i<int(var_score.size()) ; ++i)
        {
            if(var_score[i] < var_score[bestIndex]) // minimize the variance
                bestIndex = i;
        }

        // reorder the points to align them
        eastl::vector<eastl::pair<vec3,vec2>> old(std::move(top));
        top.resize(base.size());
        for(int i=0 ; i<int(base.size()) ; ++i)
            top[i] = old[(i+bestIndex)%base.size()];

    }


}
