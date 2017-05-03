#include "LTree2.h"

namespace tim
{

LTree2::Parameter LTree2::getPredefinedTree(PredefinedTree tree)
{
    LTree2::Parameter p;
    switch(tree)
    {
        case TREE_1:
            p.nbTrunkStep = 5;
            p.trunkStepSize = LTree2::GaussPDF(1, 0.2, {0.9, 1.3});
            p.trunkStepSizeDecay = LTree2::GaussPDF(0.9, 0.02, {0.8, 1});
            p.trunkAngle = LTree2::GaussPDF(7, 2, {0, 15});

            p.depth = 4;
            p.curveResolution = 0.2;

            p.branchAngle = LTree2::GaussPDF(43, 3, {30, 50});
            p.firstBranchAngleCoef = LTree2::GaussPDF(0.4, 0.08, {0.1, 0.7});
            p.branchSplitDensity = {0.3, 1, 2, 0.5};
            p.branchEarlyTermination = {0, 0, 0, 0.5};
            p.branchSplitNoise = 0.5;

            p.initialBranchSize = 0.8;
            p.branchSizeDecay = LTree2::GaussPDF(0.88, 0.005, {0.85, 0.91});
            p.branchSizeCoef = LTree2::GaussPDF(1, 0.05, {0.8, 1.2});
            p.branchSizeStopThreshold = 0.2;
            p.branchSizeAlongTrunk = {0.75,0.9,1,0.85};

            p.trunkBranchDensity = LTree2::GaussPDF(1.5, 0.1, {0, 3});
            p.trunkBranchRange = {1,99};
            p.extraBranchSpacing = 0.15;

            p.branchJitter = LTree2::GaussPDF(0.05, 0.01, {0.02, 0.07});
            p.curvatureForce = {0,0,-0.5};
            p.curvatureThicknessResistance = {0,3};
            p.naturalBranchBending = LTree2::GaussPDF(0.5, 0.05, {0, 1});
        break;

        case TREE_2:
            p.nbTrunkStep = 1;
            p.trunkStepSize = LTree2::GaussPDF(1.5, 0.3, {1.2, 1.7});
            p.trunkStepSizeDecay = LTree2::GaussPDF(1, 1, {1, 1});
            p.trunkAngle = LTree2::GaussPDF(5, 0.5, {0, 8});

            p.depth = 5;
            p.curveResolution = 0.2;

            p.branchAngle = LTree2::GaussPDF(40, 3, {30, 50});
            p.firstBranchAngleCoef = LTree2::GaussPDF(0.75, 0.08, {0.5, 1});
            p.branchSplitDensity = {0, 1, 2, 0.1};
            p.branchEarlyTermination = {0, 0, 0, 0, 0.3};
            p.branchSplitNoise = 0.5;

            p.initialBranchSize = 0.8;
            p.branchSizeDecay = LTree2::GaussPDF(0.92, 0.003, {0.88, 0.96});
            p.branchSizeCoef = LTree2::GaussPDF(1.1, 0.03, {1, 1.2});
            p.branchSizeStopThreshold = 0;
            p.branchSizeAlongTrunk = {1};

            p.trunkBranchDensity = 0;
            p.trunkBranchRange = {0,99};
            p.extraBranchSpacing = 0.1;
            p.extraBranchDensity = LTree2::GaussPDF(0.5, 0.01, {0.4, 0.6});

            p.branchJitter = LTree2::GaussPDF(0.05, 0.01, {0.02, 0.07});
            p.curvatureForce = {0,0,-1};
            p.curvatureThicknessResistance = {0,3};
            p.naturalBranchBending = LTree2::GaussPDF(0.3, 0.04, {0, 0.8});

            p.meshing.trunkThickness = 0.15;
            p.meshing.trunkThicknessDecay = {0.7, 0.75};
            p.meshing.alongTrunkThicknessDecay = {0.75, 0.75};
            p.meshing.initialBranchThickness = {0.08,0.1};
            p.meshing.branchThicknessDecay = {0.64, 0.72};
            p.meshing.alongBranchThicknessDecay = {0.65, 0.7};
        break;

        case TREE_3:
            p.nbTrunkStep = 1;
            p.trunkStepSize = LTree2::GaussPDF(1.5, 0.3, {1.2, 1.7});
            p.trunkStepSizeDecay = LTree2::GaussPDF(1, 1, {1, 1});
            p.trunkAngle = LTree2::GaussPDF(2, 0.01, {0, 4});

            p.depth = 5;
            p.curveResolution = 0.2;

            p.branchAngle = LTree2::GaussPDF(35, 0.5, {30, 45});
            p.firstBranchAngleCoef = LTree2::GaussPDF(0.7, 0.03, {0.6, 1});
            p.branchSplitDensity = {0, 1, 2, 0};
            p.branchEarlyTermination = {0, 0, 0, 0, 0.4};
            p.branchSplitNoise = 0.5;

            p.initialBranchSize = 1;
            p.branchSizeDecay = 0.9;
            p.branchSizeCoef = 1;
            p.branchSizeStopThreshold = 0;
            p.branchSizeAlongTrunk = {1};

            p.trunkBranchDensity = 0;
            p.trunkBranchRange = {0,99};
            p.extraBranchSpacing = 0.1;
            p.extraBranchDensity = 0;

            p.branchJitter = LTree2::GaussPDF(0.05, 0.005, {0, 0.08});
            p.curvatureForce = {0,0,-0.5};
            p.curvatureThicknessResistance = {0,2};
            p.naturalBranchBending = LTree2::GaussPDF(0.2, 0.02, {0, 0.4});

            p.meshing.trunkThickness = 0.25;
            p.meshing.trunkThicknessDecay = {0.8, 0.8};
            p.meshing.alongTrunkThicknessDecay = {0.8, 0.9};
            p.meshing.initialBranchThickness = {0.12,0.15};
            p.meshing.branchThicknessDecay = {0.55, 0.65};
            p.meshing.alongBranchThicknessDecay = {0.65, 0.64};
        break;

    }
    return p;
}

}
