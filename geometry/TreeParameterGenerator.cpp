#include "LTree.h"

namespace tim
{

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

            p.meshing.trunkThickness = 0.15f;
            p.meshing.trunkThicknessDecay = {0.7f, 0.75f};
            p.meshing.alongTrunkThicknessDecay = {0.75f, 0.75f};
            p.meshing.initialBranchThickness = {0.08f,0.1f};
            p.meshing.branchThicknessDecay = {0.64f, 0.72f};
            p.meshing.alongBranchThicknessDecay = {0.65f, 0.7f};
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

            p.meshing.trunkThickness = 0.25f;
            p.meshing.trunkThicknessDecay = {0.8f, 0.8f};
            p.meshing.alongTrunkThicknessDecay = {0.8f, 0.9f};
            p.meshing.initialBranchThickness = {0.12f,0.15f};
            p.meshing.branchThicknessDecay = {0.55f, 0.65f};
            p.meshing.alongBranchThicknessDecay = {0.65f, 0.64f};
        break;

    }
    return p;
}

}
