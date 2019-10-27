#include "LTree.h"

namespace tim
{

LTree::MeshingParameter LTree::getPredefinedMeshing(PredefinedMeshing meshing)
{
	LTree::MeshingParameter p;

	switch (meshing)
	{
	case SMALL:
		p.trunkThickness = 0.1f;
		p.trunkThicknessDecay = { 0.9f, 0.95f };
		p.alongTrunkThicknessDecay = { 0.92f, 0.95f };
		p.initialBranchThickness = { 0.05f,0.08f };
		p.branchThicknessDecay = { 0.85f, 0.95f };
		p.alongBranchThicknessDecay = { 0.85f, 0.95f };
		return p;

	case MEDIUM:
		p.trunkThickness = 0.15f;
		p.trunkThicknessDecay = { 0.7f, 0.8f };
		p.alongTrunkThicknessDecay = { 0.75f, 0.75f };
		p.initialBranchThickness = { 0.08f,0.1f };
		p.branchThicknessDecay = { 0.66f, 0.8f };
		p.alongBranchThicknessDecay = { 0.7f, 0.75f };
		return p;

	case BIG:
		p.trunkThickness = 0.2f;
		p.trunkThicknessDecay = { 0.8f, 0.85f };
		p.alongTrunkThicknessDecay = { 0.75f, 0.75f };
		p.initialBranchThickness = { 0.1f,0.13f };
		p.branchThicknessDecay = { 0.6f, 0.7f };
		p.alongBranchThicknessDecay = { 0.65f, 0.7f };
		return p;

	case VBIG:
		p.trunkThickness = 0.3f;
		p.trunkThicknessDecay = { 0.82f, 0.90f };
		p.alongTrunkThicknessDecay = { 0.82f, 0.82f };
		p.initialBranchThickness = { 0.17f,0.2f };
		p.branchThicknessDecay = { 0.75f, 0.82f };
		p.alongBranchThicknessDecay = { 0.75f, 0.8f };
		return p;
	}

	return p;
}

LTree::Parameter LTree::getPredefinedTree(PredefinedTree tree)
{
    LTree::Parameter p;
    switch(tree)
    {
        case TREE_1:
            p.nbTrunkStep = 5;
            p.trunkStepSize = LTree::GaussPDF(1, 0.2f, {0.9f, 1.3f});
            p.trunkStepSizeDecay = LTree::GaussPDF(0.9f, 0.02f, {0.8f, 1});
            p.trunkAngle = LTree::GaussPDF(7, 2, {0, 15});

            p.depth = 4;
            p.curveResolution = 0.2f;

            p.branchAngle = LTree::GaussPDF(43, 3, {30, 50});
            p.firstBranchAngleCoef = LTree::GaussPDF(0.4f, 0.08f, {0.1f, 0.7f});
            p.branchSplitDensity = {0.3f, 1, 2, 0.5f};
            p.branchEarlyTermination = {0, 0, 0, 0.5f};
            p.branchSplitNoise = 0.5f;

            p.initialBranchSize = 0.8f;
            p.branchSizeDecay = LTree::GaussPDF(0.88f, 0.005f, {0.85f, 0.91f});
            p.branchSizeCoef = LTree::GaussPDF(1, 0.05f, {0.8f, 1.2f});
            p.branchSizeStopThreshold = 0.2f;
            p.branchSizeAlongTrunk = {0.75f,0.9f,1,0.85f};

            p.trunkBranchDensity = LTree::GaussPDF(1.5f, 0.1f, {0, 3});
            p.trunkBranchRange = {1,99};
            p.extraBranchSpacing = 0.15f;

            p.branchJitter = LTree::GaussPDF(0.05f, 0.01f, {0.02f, 0.07f});
            p.curvatureForce = {0,0,-0.5f};
            p.curvatureThicknessResistance = {0,3};
            p.naturalBranchBending = LTree::GaussPDF(0.5f, 0.05f, {0, 1});

			p.meshing = getPredefinedMeshing(MEDIUM);
        break;

        case TREE_2:
            p.nbTrunkStep = 1;
            p.trunkStepSize = LTree::GaussPDF(1.5f, 0.3f, {1.2f, 1.7f});
            p.trunkStepSizeDecay = LTree::GaussPDF(1, 1, {1, 1});
            p.trunkAngle = LTree::GaussPDF(5, 0.5f, {0, 8});

            p.depth = 5;
            p.curveResolution = 0.2f;

            p.branchAngle = LTree::GaussPDF(40, 3, {30, 50});
            p.firstBranchAngleCoef = LTree::GaussPDF(0.75f, 0.08f, {0.5, 1});
            p.branchSplitDensity = {0, 1, 2, 0.1f};
            p.branchEarlyTermination = {0, 0, 0, 0, 0.3f};
            p.branchSplitNoise = 0.5f;

            p.initialBranchSize = 0.8f;
            p.branchSizeDecay = LTree::GaussPDF(0.92f, 0.003f, {0.88f, 0.96f});
            p.branchSizeCoef = LTree::GaussPDF(1.1f, 0.03f, {1, 1.2f});
            p.branchSizeStopThreshold = 0;
            p.branchSizeAlongTrunk = {1};

            p.trunkBranchDensity = 0;
            p.trunkBranchRange = {0,99};
            p.extraBranchSpacing = 0.1f;
            p.extraBranchDensity = LTree::GaussPDF(0.5f, 0.01f, {0.4f, 0.6f});

            p.branchJitter = LTree::GaussPDF(0.05f, 0.01f, {0.02f, 0.07f});
            p.curvatureForce = {0,0,-1};
            p.curvatureThicknessResistance = {0,3};
            p.naturalBranchBending = LTree::GaussPDF(0.3f, 0.04f, {0, 0.8f});

			p.meshing = getPredefinedMeshing(MEDIUM);
        break;

        case TREE_3:
            p.nbTrunkStep = 1;
            p.trunkStepSize = LTree::GaussPDF(1.5f, 0.3f, {1.2f, 1.7f});
            p.trunkStepSizeDecay = LTree::GaussPDF(1, 1, {1, 1});
            p.trunkAngle = LTree::GaussPDF(2, 0.01f, {0, 4});

            p.depth = 5;
            p.curveResolution = 0.2f;

            p.branchAngle = LTree::GaussPDF(35, 0.5f, {30, 45});
            p.firstBranchAngleCoef = LTree::GaussPDF(0.7f, 0.03f, {0.6f, 1});
            p.branchSplitDensity = {0, 1, 2, 0};
            p.branchEarlyTermination = {0, 0, 0, 0, 0.4f};
            p.branchSplitNoise = 0.5;

            p.initialBranchSize = 1;
            p.branchSizeDecay = 0.9f;
            p.branchSizeCoef = 1;
            p.branchSizeStopThreshold = 0;
            p.branchSizeAlongTrunk = {1};

            p.trunkBranchDensity = 0;
            p.trunkBranchRange = {0,99};
            p.extraBranchSpacing = 0.1f;
            p.extraBranchDensity = 0;

            p.branchJitter = LTree::GaussPDF(0.05f, 0.005f, {0, 0.08f});
            p.curvatureForce = {0,0,-0.5f};
            p.curvatureThicknessResistance = {0,2};
            p.naturalBranchBending = LTree::GaussPDF(0.2f, 0.02f, {0, 0.4f});

			p.meshing = getPredefinedMeshing(BIG);
        break;

		case TREE_4:
			p.nbTrunkStep = 5;
			p.trunkStepSize = LTree::GaussPDF(1, 0.2f, { 0.9f, 1.3f });
			p.trunkStepSizeDecay = LTree::GaussPDF(0.9f, 0.02f, { 0.8f, 1 });
			p.trunkAngle = LTree::GaussPDF(7, 2, { 0, 15 });

			p.depth = 3;
			p.curveResolution = 0.2f;

			p.branchAngle = LTree::GaussPDF(53, 3, { 42, 65 });
			p.firstBranchAngleCoef = LTree::GaussPDF(0.25f, 0.08f, { 0.1f, 0.5f });
			p.branchSplitDensity = { 0, 3, 3, 1 };
			p.branchEarlyTermination = { 0, 0, 0, 0.5f };
			p.branchSplitNoise = 0.5f;

			p.initialBranchSize = 1.f;
			p.branchSizeDecay = LTree::GaussPDF(0.94f, 0.003f, { 0.91f, 0.96f });
			p.branchSizeCoef = LTree::GaussPDF(1, 0.02f, { 0.9f, 1.1f });
			p.branchSizeStopThreshold = 0.2f;
			p.branchSizeAlongTrunk = { 0.8f,1,1,0.8f };

			p.trunkBranchDensity = LTree::GaussPDF(1.5f, 0.1f, { 1, 2 });
			p.trunkBranchRange = { 1,99 };
			p.extraBranchSpacing = 0.15f;

			p.branchJitter = LTree::GaussPDF(0.045f, 0.01f, { 0.02f, 0.07f });
			p.curvatureForce = { 0,0,-0.3f };
			p.curvatureThicknessResistance = { 0,3 };
			p.naturalBranchBending = LTree::GaussPDF(0.5f, 0.05f, { 0, 1 });

			p.meshing = getPredefinedMeshing(BIG);
			break;

		case BIG_TREE1:
			p.nbTrunkStep = 5;
			p.trunkStepSize = LTree::GaussPDF(2.2f, 0.3f, { 1.9f, 2.5f });
			p.trunkStepSizeDecay = LTree::GaussPDF(0.975f, 0.01f, { 0.95f, 1.02f });
			p.trunkAngle = LTree::GaussPDF(3, 0.1f, { 0, 5 });

			p.depth = 4;
			p.curveResolution = 0.4f;

			p.branchAngle = LTree::GaussPDF(70, 5, { 50, 90 });
			p.firstBranchAngleCoef = vec2({0.7f,1});
			p.branchSplitDensity = { 0, 1, 2, 0.5f };
			p.branchEarlyTermination = { 0,0,0,0 };
			p.branchSplitNoise = 0.3f;

			p.initialBranchSize = 2;
			p.branchSizeDecay = LTree::GaussPDF(0.8f, 0.005f, { 0.78f, 0.91f });
			p.branchSizeCoef = LTree::GaussPDF(1, 0.02f, { 0.9f, 1.1f });
			p.branchSizeStopThreshold = 0.3f;
			p.branchSizeAlongTrunk = { 0,0, 0,0, 0.5f,0.75f, 0.85f, 1 };

			p.trunkBranchDensity = vec2(1.5,3);
			p.trunkBranchRange = { 3,99 };
			p.extraBranchSpacing = 0.3f;

			p.branchJitter = LTree::GaussPDF(0.02f, 0.005f, { 0.01f, 0.05f });
			p.curvatureForce = { 0,0,-1 };
			p.curvatureThicknessResistance = { 0,3 };
			p.naturalBranchBending = LTree::GaussPDF(0.5f, 0.05f, { 0, 1 });

			p.meshing = getPredefinedMeshing(VBIG);
			break;

		case BIG_TREE2:
			p.nbTrunkStep = 5;
			p.trunkStepSize = LTree::GaussPDF(2.2f, 0.3f, { 1.9f, 2.5f });
			p.trunkStepSizeDecay = LTree::GaussPDF(0.975f, 0.01f, { 0.95f, 1.02f });
			p.trunkAngle = LTree::GaussPDF(3, 0.1f, { 0, 5 });

			p.depth = 4;
			p.curveResolution = 0.4f;

			p.branchAngle = LTree::GaussPDF(40, 1, { 35, 55 });
			p.firstBranchAngleCoef = vec2({ 0.7f,1 });
			p.branchSplitDensity = { 0, 0.5, 2, 0.4f };
			p.branchEarlyTermination = { 0,0,0,0.3f };
			p.branchSplitNoise = 0.3f;

			p.initialBranchSize = 2;
			p.branchSizeDecay = LTree::GaussPDF(0.7f, 0.005f, { 0.6f, 0.8f });
			p.branchSizeCoef = LTree::GaussPDF(1, 0.02f, { 0.9f, 1.1f });
			p.branchSizeStopThreshold = 0.3f;
			p.branchSizeAlongTrunk = { 0,0, 0,0, 0.3f,0.5f, 0.8f, 1 };

			p.trunkBranchDensity = vec2(1.5, 3);
			p.trunkBranchRange = { 3,99 };
			p.extraBranchSpacing = 0.3f;

			p.branchJitter = LTree::GaussPDF(0.02f, 0.005f, { 0.01f, 0.05f });
			p.curvatureForce = { 0,0,1 };
			p.curvatureThicknessResistance = { 0,2 };
			p.naturalBranchBending = LTree::GaussPDF(0.5f, 0.05f, { 0, 1 });

			p.meshing = getPredefinedMeshing(VBIG);
			break;

		case BIG_TREE3:
			p.nbTrunkStep = 5;
			p.trunkStepSize = LTree::GaussPDF(2.25f, 0.3f, { 2.f, 2.5f });
			p.trunkStepSizeDecay = LTree::GaussPDF(0.975f, 0.01f, { 0.95f, 1.02f });
			p.trunkAngle = LTree::GaussPDF(3, 0.1f, { 0, 5 });

			p.depth = 3;
			p.curveResolution = 0.4f;

			p.branchAngle = LTree::GaussPDF(75, 1, { 70, 85 });
			p.firstBranchAngleCoef = vec2( 0.9f,1 );
			p.branchSplitDensity = { 0, 1, 2, 0.5f };
			p.branchEarlyTermination = { 0,0,0,0.3f };
			p.branchSplitNoise = 0.3f;

			p.initialBranchSize = 3;
			p.branchSizeDecay = LTree::GaussPDF(0.5f, 0.005f, { 0.4f, 0.6f });
			p.branchSizeCoef = LTree::GaussPDF(1, 0.02f, { 0.8f, 1.2f });
			p.branchSizeStopThreshold = 0.25f;
			p.branchSizeAlongTrunk = { 1 };

			p.trunkBranchDensity = vec2(8, 10);
			p.trunkBranchRange = { 4.5f,99 };
			p.extraBranchSpacing = 0.2f;

			p.branchJitter = LTree::GaussPDF(0.02f, 0.005f, { 0.01f, 0.05f });
			p.curvatureForce = { 0,0,-1 };
			p.curvatureThicknessResistance = { 3,0.1f };
			p.naturalBranchBending = LTree::GaussPDF(0.02f, 0.003f, { 0, 0.04f });

			p.meshing = getPredefinedMeshing(VBIG);
			p.meshing.branchThicknessDecay *= 0.9;
			p.meshing.alongBranchThicknessDecay *= 0.9;
			p.meshing.initialBranchThickness *= 0.7;
			break;
    }
    return p;
}

}
