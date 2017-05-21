#pragma once
#include "Mesh.h"

namespace tim
{

struct Geometry
{
    Geometry() = delete;

    template<class MeshType = Mesh>
    static MeshType generateCubeSphere(uint resolution, float radius=1, bool triangulate = false)
    {
        MeshType plan = MeshType::generateGrid(vec2(1,1), {resolution, resolution}, ImageAlgorithm<float>(), 0, triangulate);
        MeshType result;

        result += plan.translated(vec3(0,0,0.5));
        result += plan.translated(vec3(0,0,0.5)).scaled(vec3(1,1,-1)).invertFaces();

        result += plan.rotated(mat3::RotationX(toRad(-90))).translated(vec3(0,0.5,0));
        result += plan.rotated(mat3::RotationX(toRad(90))).translated(vec3(0,-0.5,0));

        result += plan.rotated(mat3::RotationY(toRad(90))).translated(vec3(0.5,0,0));
        result += plan.rotated(mat3::RotationY(toRad(-90))).translated(vec3(-0.5,0,0));

        return result.mapVertices([=](vec3 v) { return v.normalized()*radius; });
    }
};

}

