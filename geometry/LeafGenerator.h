#ifndef LEAFGENERATOR_H
#define LEAFGENERATOR_H

#include "math/Vector.h"
#include "core/type.h"
#include "Mesh.h"
#include "math/SampleFunction.h"

namespace tim
{

    class LeafGenerator
    {
    public:
        struct Parameter
        {
            uint resX = 8;
            vec2 size = {1, 0.5};
            float innerCurvature = 0.1;
            float outerCurvature = 0.2;
            float curvature = 0;
            bool triangulate = true;
            bool smoothAlongY = true;
        };

        LeafGenerator();

        void generate(const Parameter&);

        const UVMesh& mesh() const { return _mesh; }

    private:
        UVMesh _mesh;

        void generateWithSplit(const Parameter&);

        static SampleFunction leafFunction(int);
    };

}

#endif // LEAFGENERATOR_H
