#pragma once

#include "Curve.h"
#include "Mesh.h"
#include "math/Quaternion.h"
#include "math/PDF.h"
#include "math/SampleFunction.h"
#include <EASTL/deque.h>

#include <random>

namespace tim
{
    class LTree
    {
    public:
        using GaussPDF = TruncatedGaussianPDF<>;

        struct MeshingParameter
        {
            float trunkThickness = 0.2f;
            vec2 alongTrunkThicknessDecay = {0.8f, 0.8f};
            vec2 trunkThicknessDecay = {0.75f,0.8f};

            vec2 alongBranchThicknessDecay = {0.7f, 0.9f};
            vec2 initialBranchThickness = {0.08f, 0.08f};
            vec2 branchThicknessDecay = {0.7f,0.9f};

			/* methods */
			MeshingParameter& scale(float);
			MeshingParameter& scaleDecay(tim::vec2);

			static MeshingParameter interpolate(const MeshingParameter& m1, const MeshingParameter& m2, float);
        };

        struct Parameter
        {
            int nbTrunkStep = 4;
            GaussPDF trunkAngle = vec2(0,10);
            GaussPDF trunkStepSize = vec2(1, 1);
            GaussPDF trunkStepSizeDecay = vec2(0.9, 1);

            int depth = 4;
            float curveResolution = 0;

            GaussPDF branchAngle = vec2(10,40);
            GaussPDF firstBranchAngleCoef = vec2(1,1);
            eastl::vector<float> branchSplitDensity = {0,1,1};
            eastl::vector<float> branchEarlyTermination = {0,0,0,0.5};
            float branchSplitNoise = 0.5f;

            float initialBranchSize = 0.5f;
            GaussPDF branchSizeDecay = vec2(0.7, 0.7);
            GaussPDF branchSizeCoef = vec2(1,1);
            float branchSizeStopThreshold = 0.2f;
            SampleFunction branchSizeAlongTrunk={1};

            vec2 trunkBranchRange = {0.3f,1};
            GaussPDF trunkBranchDensity = 0;
            GaussPDF extraBranchDensity = 0;
            float extraBranchSpacing = 0.2f;

            GaussPDF branchJitter = 0;
            GaussPDF naturalBranchBending = 0;

            vec3 curvatureForce = vec3(0,0,0);
            vec2 curvatureThicknessResistance = vec2(0,1); // x=constante,y=quadratic

            MeshingParameter meshing;

			/* methods */
			Parameter& alterate(int seed, float amount = 1);

			static eastl::vector<float> interpolate(const eastl::vector<float>&, const eastl::vector<float>&, float);
			static Parameter interpolate(const Parameter&, const Parameter&, float);
            void print() const;
        };

        struct LeafParameter
        {
            BaseMesh leaf;
            uint depth = 0;
            GaussPDF density = 3;
            GaussPDF orientation = vec2(-PI/2, PI/2);
            GaussPDF tilt = vec2(-PI/4, PI/4);
            GaussPDF scale = 1;

			static LeafParameter gen(int seed, int sizeCategorie);
        };

        LTree(Parameter, int seed=42);
        ~LTree() = default;

        LTree(const LTree&) = default;
        LTree(LTree&&) = default;

        LTree& operator=(const LTree&) = default;
        LTree& operator=(LTree&&) = default;

        void exportOBJ(eastl::string) const;
        Mesh generateMesh(int resolution = 8) const;
        UVMesh generateUVMesh(int resolution = 8) const;
        Mesh generateLeaf(const LeafParameter&) const;

        enum PredefinedTree { TREE_1, TREE_2, TREE_3, TREE_4,
							  BIG_TREE1, BIG_TREE2, BIG_TREE3 };
        static Parameter getPredefinedTree(PredefinedTree);

		enum PredefinedMeshing { SMALL, MEDIUM, BIG, VBIG };
		static MeshingParameter getPredefinedMeshing(PredefinedMeshing);

    private:
        struct Node
        {
            Node* parent;

            Curve* curve;
            uivec2 range;

            Node* child;
            eastl::vector<Node*> nodes;
            //float branchPosition, branchAngle, thickness;
        };

        Node* _root = nullptr;

        eastl::deque<Node, EASTLAllocatorType, 256> _nodePool;
        eastl::deque<Curve, EASTLAllocatorType, 256> _curvePool;

        mutable std::mt19937 _randEngine;
        mutable std::uniform_real_distribution<float> _random;

    private:
        static void accumulateMesh(Mesh&, const Node*, int depth);
        void generateMeshRec(Node*, Mesh&, int, int) const;
        void generateUVMeshRec(Node*, UVMesh&, int, int) const;

        struct GenParam
        {
            float branchSize;
            float thickness;
            bool needNewCurve=true;
            bool isTrunk=true;
            int inBranchDepth=0;
            int totalDepth=0;
            float trunkDecay=1;
        };

        Node* generateBranchRec(const Parameter&, Node* parent, vec3 position, vec3 direction, GenParam detailParam);

        uint generateLeafRec(const LeafParameter&, Node*, Mesh&) const;

        static vec3 sampleSubCurve(float sample, Curve& curve, uivec2 range, vec3& dir, float& thickness);

        static float mapRange(float, vec2);
        int randInt(ivec2) const;
        template<class T> int randFromDensity(const T&) const;
        static vec3 genDir(vec3 baseDir, float theta, float phi);

    };

    inline float LTree::mapRange(float x, vec2 range)
    {
        return (range[1] - range[0])*x + range[0];
    }

    template<class T> int LTree::randFromDensity(const T& density) const
    {
        float x = _random(_randEngine);
        float acc = 0;
        for(size_t i=0 ; i<density.size() ; ++i)
        {
            acc += density[i];
            if(x < acc) return int(i);
        }
        return 0;
    }
}
