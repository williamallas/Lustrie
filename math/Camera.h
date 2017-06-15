#pragma once

#include "Vector.h"
#include "Matrix.h"

namespace tim
{
    struct Camera
    {
        vec3 pos, dir={0,1,0}, up={0,0,1};
        float fov=70, ratio=1;
        vec2 clipDist={1,100};

		bool useRawMat = false;
		mat4 raw_view = mat4::IDENTITY();
		mat4 raw_proj = mat4::IDENTITY();

        vec3 computeCenter() const
        {
            return pos+(dir-pos).normalized()*(clipDist.y()-clipDist.x())*0.5;
        }
    };

}
