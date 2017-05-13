
#include "LTree.h"

namespace tim
{

LTree::LTree(Parameter parameter, int seed) : _randEngine(seed), _random(0,1)
{
    float acc=0;
    for(auto x : parameter.branchSplitDensity)
        acc += x;
    for(auto& x : parameter.branchSplitDensity)
        x /= acc;

    parameter.nbTrunkStep = eastl::max(1, parameter.nbTrunkStep);

    while(parameter.branchEarlyTermination.size() < size_t(parameter.depth))
        parameter.branchEarlyTermination.push_back(parameter.branchEarlyTermination.empty() ? 0:parameter.branchEarlyTermination.back());

    GenParam detail;
    detail.branchSize = 0; // initially the trunk is generated
    detail.thickness = parameter.meshing.trunkThickness;
    _root = generateBranchRec(parameter, nullptr, vec3(0,0,0), vec3(0,0,1), detail);
}

Mesh LTree::generateMesh(int resolution) const
{
    Mesh mesh;
    generateMeshRec(_root, mesh, resolution, 0);
    return mesh;
}

UVMesh LTree::generateUVMesh(int resolution) const
{
    UVMesh mesh;
    generateUVMeshRec(_root, mesh, resolution, 0);
    return mesh;
}

Mesh LTree::generateLeaf(const LeafParameter& leaf) const
{
    Mesh m;
    generateLeafRec(leaf, _root, m);
    return m;
}

void LTree::exportOBJ(eastl::string filename) const
{
    Mesh acc;
    accumulateMesh(acc, _root, 0);

    acc.exportToObj(filename);
}

LTree::Node* LTree::generateBranchRec(const Parameter& param, Node* parent, vec3 position, vec3 direction, GenParam detailParam)
{
    Node* node = &_nodePool.push_back();
    node->parent = parent;
    bool isTrunk = detailParam.isTrunk;

    // initialize node
    if(detailParam.needNewCurve)
    {
        node->curve = &_curvePool.push_back();
        node->curve->addPoint(position, detailParam.thickness);
        node->range.x() = 0;
    }
    else
    {
        node->curve = parent->curve;
        node->range.x() = node->curve->numPoints()-1;
    }

    // generate curve
    vec3 pointAtStart = node->curve->point(node->range.x());
    float curveLength = isTrunk ? (param.trunkStepSize(_randEngine) * detailParam.trunkDecay) :
                                  (detailParam.branchSize * param.branchSizeCoef(_randEngine));
    vec3 pointAtEnd = pointAtStart + direction*curveLength;

    vec2 alongDecay = isTrunk ? param.meshing.alongTrunkThicknessDecay : param.meshing.alongBranchThicknessDecay;
    float endThickness = detailParam.thickness*mapRange(_random(_randEngine), alongDecay);

    int curveResolution = (param.curveResolution == 0) ? 1:int(0.5f + curveLength / param.curveResolution);
    if(curveResolution < 2 && ((isTrunk  && detailParam.inBranchDepth+1 == param.nbTrunkStep) ||
                               (!isTrunk && detailParam.totalDepth+1 == param.depth)))
        curveResolution = 2;
    else if(curveResolution <= 0)
        curveResolution = 1;

    node->range.y() = node->range.x() + curveResolution;

    vec3 localDir = (pointAtEnd - pointAtStart) / curveResolution;
    float localDirLength = localDir.length();

    vec3 curPts = pointAtStart;
    float tenseForceFactor = param.naturalBranchBending(_randEngine) * (!isTrunk ? 1 : 0.3);

    vec3 force = vec3(_random(_randEngine), _random(_randEngine), _random(_randEngine))-vec3(0.5,0.5,0.5);

    float sizeBranch = curveLength; // (pointAtEnd - pointAtStart).length();
    Quat quat = Quat::from_axis_angle(localDir.cross(force).normalized(), sizeBranch * tenseForceFactor * 0.2);

    for(int i=0 ; i<curveResolution ; ++i)
    {
        curPts += localDir;
        localDir = quat(localDir);
        float coef = sizeBranch / (1000*param.curvatureThicknessResistance.y()*detailParam.thickness*detailParam.thickness + param.curvatureThicknessResistance.x());
        localDir = (localDir+param.curvatureForce * 0.1f*coef).resized(localDirLength);

        localDir = Quat::from_axis_angle(localDir.cross(vec3(_random(_randEngine), _random(_randEngine), _random(_randEngine))-vec3(0.5,0.5,0.5)).normalized(),
            param.branchJitter(_randEngine)*PI*(isTrunk?(float(i)/curveResolution):1))(localDir);

        node->curve->addPoint(curPts, interpolate(detailParam.thickness, endThickness, float(i+1)/curveResolution));
    }
    pointAtEnd = curPts;

    // gen the trunk if needed
    bool trunkContinue = isTrunk && detailParam.inBranchDepth+1 < param.nbTrunkStep;
    if(trunkContinue)
    {
        vec3 dir = genDir(node->curve->computeDirection(node->range.x()),
                          _random(_randEngine)*TAU,
                          toRad(param.trunkAngle(_randEngine)));

        GenParam newGenParam = detailParam;
        newGenParam.thickness = std::min(endThickness, detailParam.thickness * mapRange(_random(_randEngine), param.meshing.trunkThicknessDecay));
        newGenParam.trunkDecay *= param.trunkStepSizeDecay(_randEngine);

        newGenParam.totalDepth = 0;
        newGenParam.inBranchDepth++;
        newGenParam.needNewCurve = false;
        newGenParam.isTrunk = true;

        node->child = generateBranchRec(param, node, pointAtEnd, dir, newGenParam);
    }
    // now the full curve is generated

    // split branchs
    if(detailParam.totalDepth+1 < param.depth && _random(_randEngine) > param.branchEarlyTermination[detailParam.totalDepth+1])
    {
        vec3 baseDir = node->curve->computeDirection(node->range.x());
        float theta = _random(_randEngine)*TAU;
        float phi = toRad(param.branchAngle(_randEngine) * (trunkContinue ? 1:param.firstBranchAngleCoef(_randEngine)));

        vec3 dir = genDir(baseDir, theta, phi);

        float coef = isTrunk ? param.branchSizeAlongTrunk(float(detailParam.inBranchDepth) / (param.nbTrunkStep <= 1 ? 1:(param.nbTrunkStep-1))) : 1;

        GenParam newGenParam = detailParam;
        newGenParam.needNewCurve = trunkContinue;
        newGenParam.isTrunk = false;
        newGenParam.totalDepth = isTrunk ? 0 : newGenParam.totalDepth+1;
        newGenParam.inBranchDepth = isTrunk ? 0 : newGenParam.inBranchDepth+1;
        newGenParam.branchSize = (isTrunk ? param.initialBranchSize : newGenParam.branchSize*param.branchSizeDecay(_randEngine)) * coef;
        newGenParam.thickness = isTrunk ? mapRange(_random(_randEngine), param.meshing.initialBranchThickness) :
                                          mapRange(_random(_randEngine), param.meshing.branchThicknessDecay)*detailParam.thickness;
        newGenParam.thickness = std::min(endThickness, newGenParam.thickness) * coef;

        if(newGenParam.branchSize > param.branchSizeStopThreshold)
        {
            Node* firstNewSplittedBranch = generateBranchRec(param, node, pointAtEnd, dir, newGenParam);
            if(!trunkContinue)
                node->child = firstNewSplittedBranch;
            else
                node->nodes.push_back(firstNewSplittedBranch);

            newGenParam.inBranchDepth=0;
            newGenParam.needNewCurve=true;

            int nbSplit = randFromDensity(param.branchSplitDensity) + 1;
            for(int i=1 ; i<nbSplit ; ++i)
            {
                vec3 dir = genDir(baseDir, fmodf(theta + (i*TAU / nbSplit) + (param.branchSplitNoise*(_random(_randEngine)-0.5f)*TAU / nbSplit), TAU),
                                  toRad(param.branchAngle(_randEngine)));

                newGenParam.thickness = std::min(endThickness, newGenParam.thickness) * coef;
                newGenParam.branchSize = (isTrunk ? param.initialBranchSize : newGenParam.branchSize*param.branchSizeDecay(_randEngine)) * coef;

                if(newGenParam.branchSize > param.branchSizeStopThreshold)
                    node->nodes.push_back( generateBranchRec(param, node, pointAtEnd, dir, newGenParam));
            }
        }

        if(sizeBranch < 2*param.extraBranchSpacing)
            return node;

        // create new extra branches
        {
        float nbBranchf = (isTrunk ? param.trunkBranchDensity:param.extraBranchDensity)(_randEngine) * (sizeBranch-param.extraBranchSpacing);
        int nbBranch = int(nbBranchf) + (_random(_randEngine) < modf(nbBranchf, nullptr) ? 1:0);
        eastl::vector<vec2> alreadyCreatedBranch;

        GenParam newGenParam = detailParam;
        newGenParam.needNewCurve = true;
        newGenParam.isTrunk = false;
        newGenParam.totalDepth = isTrunk ? 0 : newGenParam.totalDepth+1;
        newGenParam.inBranchDepth = isTrunk ? 0 : newGenParam.inBranchDepth+1;

        for(int i=0 ; i<nbBranch ; ++i)
        {
            vec3 branchDir;
            float atBranchThickness;
            float onBranchPosSample = _random(_randEngine);
            float onBranchPos = (param.extraBranchSpacing + onBranchPosSample * (sizeBranch-2*param.extraBranchSpacing)) / sizeBranch;

            if(isTrunk && (onBranchPos + float(detailParam.inBranchDepth)) < param.trunkBranchRange.x())
                continue;

            float coef = isTrunk ? param.branchSizeAlongTrunk(onBranchPosSample + float(detailParam.inBranchDepth)) : 1;

            vec3 branchPos = sampleSubCurve(onBranchPos, *node->curve, node->range, branchDir, atBranchThickness);

            onBranchPos *= sizeBranch;
            float theta = _random(_randEngine)*TAU;
            float phi = toRad(param.branchAngle(_randEngine));

            // check if the branch can be generated
            bool finish = false;
            for(auto v : alreadyCreatedBranch)
            {
                if( (fabsf(v.x()-onBranchPos) < param.extraBranchSpacing && ( fabsf(v.y()-theta) < TAU/5 || fabsf(v.y()-theta + TAU) < TAU/5)) )
                {
                    finish = true;
                    continue;
                }
            }

            if(finish) continue;

            alreadyCreatedBranch.push_back({onBranchPos, theta});

            newGenParam.branchSize = (isTrunk ? param.initialBranchSize : newGenParam.branchSize*param.branchSizeDecay(_randEngine)) * coef;
            newGenParam.thickness = isTrunk ? mapRange(_random(_randEngine), param.meshing.initialBranchThickness) :
                                              mapRange(_random(_randEngine), param.meshing.branchThicknessDecay)*detailParam.thickness;
            newGenParam.thickness = std::min(atBranchThickness, newGenParam.thickness) * coef;

            branchDir = genDir(branchDir, theta, phi);
            if(newGenParam.branchSize > param.branchSizeStopThreshold)
                node->nodes.push_back( generateBranchRec(param, node, branchPos, branchDir, newGenParam) );
        }
        }
    }

    return node;
}

void LTree::accumulateMesh(Mesh& acc, const LTree::Node* node, int depth)
{
    if(depth == 0)
        acc += node->curve->convertToWireMesh();

    if(node->child)
        accumulateMesh(acc, node->child, depth + 1);

    for(Node* n : node->nodes)
        accumulateMesh(acc, n, 0);
}

void LTree::generateMeshRec(Node* node, Mesh& mesh, int resolution, int depth) const
{
    if(depth == 0)
        mesh += node->curve->convertToMesh(resolution, true, true);

    if(node->child)
        generateMeshRec(node->child, mesh, resolution, depth+1);

    for(auto child : node->nodes)
        generateMeshRec(child, mesh, resolution, 0);
}

void LTree::generateUVMeshRec(Node* node, UVMesh& mesh, int resolution, int depth) const
{
    if(depth == 0)
        mesh += node->curve->convertToUVMesh(resolution, true, true);

    if(node->child)
        generateUVMeshRec(node->child, mesh, resolution, depth+1);

    for(auto child : node->nodes)
        generateUVMeshRec(child, mesh, resolution, 0);
}

uint LTree::generateLeafRec(const LeafParameter& leaf, Node* node, Mesh& acc) const
{
    uint depth = 1 << 31;
    if(node->child)
    {
        depth = generateLeafRec(leaf, node->child, acc);
        for(auto n : node->nodes)
            generateLeafRec(leaf, n, acc);
    }

    if(!node->child || depth <= leaf.depth)
    {
        float nbLeaff =  (node->curve->point(node->range.y())-node->curve->point(node->range.x())).length() * leaf.density(_randEngine);
        int nbLeaf = int(nbLeaff) + (_random(_randEngine) < modf(nbLeaff, nullptr) ? 1:0);
        for(int i=0 ; i<nbLeaf ; ++i)
        {
            vec3 dir; float thickness;
            vec3 pos = sampleSubCurve(_random(_randEngine), *(node->curve), node->range, dir, thickness);
            vec3 ortho = dir.cross(vec3(0,0,1));
            vec3 up = ortho.cross(dir);

            mat3 orientation = mat3({dir, ortho, up});
            acc += leaf.leaf.scaled(vec3::construct(leaf.scale(_randEngine)))
                            .rotated(Quat::from_axis_angle(ortho, leaf.tilt(_randEngine)))
                            .rotated(Quat::from_axis_angle(up, (_randEngine()%2==0 ? -1:1) * leaf.orientation(_randEngine)))
                            .rotated(orientation).translated(pos);
        }
    }

    if(!node->child)
        return 0;
    else
        return depth + 1;
}

vec3 LTree::genDir(vec3 baseDir, float theta, float phi)
{
    return Quat::from_axis_angle( baseDir, theta )
            (Quat::from_axis_angle( baseDir.orthoDir(), phi )
             (baseDir)).normalize();

    /*return (mat3::changeBasis(baseDir) *
            mat3::RotationZ(theta) *
            mat3::RotationX(phi) * vec3(0,0,1)).normalize();*/
}

int LTree::randInt(ivec2 r) const
{
    if(r.x() >= r.y())
        return r.x();

    return r.x() + (_randEngine() % (1+r.y()-r.x()));
}

vec3 LTree::sampleSubCurve(float sample, Curve& curve, uivec2 range, vec3& dir, float& thickness)
{
    float x = float(range.x()) + (range.y() - range.x()) * sample;

    uint ix  = uint(x);
    uint ix2 = std::max<uint>(std::min<uint>(ix+1, range[1]), range[0]);
    x = modf(x, nullptr);

    dir = interpolate(curve.computeDirection(ix), curve.computeDirection(ix2), x);
    thickness = interpolate(curve.radius(ix), curve.radius(ix2), x);
    return interpolate(curve.point(ix), curve.point(ix2), x);
}

LTree::Parameter LTree::Parameter::random(int seed)
{
    struct Detail
    {
        std::mt19937& gen;
        std::uniform_real_distribution<float> random;

        LTree::GaussPDF randGaussePDF(float a, float b, float var=0)
        { return LTree::GaussPDF(interpolate(a+random(gen)*var, b-random(gen)*var, random(gen)), (b-a) * exp(-random(gen)*3), {a,b}); }
    };

    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> random(0,1);
    Detail detail({gen, random});

    LTree::Parameter param;
    param.nbTrunkStep = std::geometric_distribution<int>(0.25)(gen);
    param.trunkAngle = detail.randGaussePDF(0, 30, 10);
    param.trunkStepSize = detail.randGaussePDF(0.3, 3.0, 1);
    param.trunkStepSizeDecay = detail.randGaussePDF(0.5, 1, 0.2);

    param.depth = (gen() %  4) + 3;
    param.curveResolution = 0.2;

    param.branchAngle = detail.randGaussePDF(15, 70, 20);
    param.firstBranchAngleCoef = detail.randGaussePDF(0, 1, 0.2);

    param.branchSplitDensity = {random(gen), 1.f+random(gen), 1.f+random(gen)*1.5f, random(gen)};
    param.branchSplitNoise = LTree::GaussPDF({0.1f,0.9f})(gen);

    param.branchEarlyTermination.resize(param.depth);
    for(size_t i=0 ; i<param.branchEarlyTermination.size() ; ++i)
        param.branchEarlyTermination[i] = (int(i) <= param.depth/2 ? 0 : (i*random(gen)/param.depth));


    param.initialBranchSize = std::uniform_real_distribution<float>(0.5,2)(gen) * param.trunkStepSize(gen);
    param.branchSizeDecay = detail.randGaussePDF(0.4, 1.f, 0.2);
    param.branchSizeCoef = detail.randGaussePDF(0.5, 2, 0.5);
    param.branchSizeStopThreshold = 0.1 + random(gen) * 0.1;

    //param.trunkBranchRange = {0.3,1};
    //param.trunkBranchDensity = 0;
    //param.extraBranchDensity = 2;
    //param.extraBranchSpacing = 0.2;

    param.branchJitter = detail.randGaussePDF(0, 0.1, 0.03);
    param.naturalBranchBending = detail.randGaussePDF(0, 0.5, 0.1);

    param.curvatureForce = vec3(0, 0, LTree::GaussPDF(0, 0.5, {-2,2})(gen));
    param.curvatureThicknessResistance = vec2(0,1); // x=constante,y=quadratic

    return param;
}

/*
void LTree::Parameter::print() const
{
    std::cout << "nbTrunkStep=" << nbTrunkStep << std::endl;
    std::cout << "trunkAngle=" << trunkAngle << std::endl;
    std::cout << "trunkStepSize=" << trunkStepSize << std::endl;
    std::cout << "trunkStepSizeDecay=" << trunkStepSizeDecay << std::endl;
    std::cout << "depth=" << depth << std::endl;
    std::cout << "curveResolution=" << curveResolution << std::endl;
    std::cout << "branchAngle=" << branchAngle << std::endl;
    std::cout << "firstBranchAngleCoef=" << firstBranchAngleCoef << std::endl;
    std::cout << "branchSplitNoise=" << branchSplitNoise << std::endl;
    std::cout << "initialBranchSize=" << initialBranchSize << std::endl;
    std::cout << "branchSizeDecay=" << branchSizeDecay << std::endl;
    std::cout << "branchSizeCoef=" << branchSizeCoef << std::endl;
    std::cout << "branchSizeStopThreshold=" << branchSizeStopThreshold << std::endl;
    std::cout << "trunkBranchRange=" << trunkBranchDensity << std::endl;
    std::cout << "trunkBranchDensity=" << trunkBranchDensity << std::endl;
    std::cout << "extraBranchDensity=" << extraBranchDensity << std::endl;
    std::cout << "extraBranchSpacing=" << extraBranchSpacing << std::endl;
    std::cout << "branchJitter=" << branchJitter << std::endl;
    std::cout << "naturalBranchBending=" << naturalBranchBending << std::endl;
    std::cout << "curvatureForce=" << curvatureForce[2] << std::endl;
    std::cout << "curvatureThicknessResistance=" << curvatureThicknessResistance << std::endl;
}
*/


}
