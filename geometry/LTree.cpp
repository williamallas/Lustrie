
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

    vec3 localDir = (pointAtEnd - pointAtStart) / (float)curveResolution;
    float localDirLength = localDir.length();

    vec3 curPts = pointAtStart;
    float tenseForceFactor = param.naturalBranchBending(_randEngine) * (!isTrunk ? 1 : 0.3f);

    vec3 force = vec3(_random(_randEngine), _random(_randEngine), _random(_randEngine))-vec3(0.5,0.5,0.5);

    float sizeBranch = curveLength; // (pointAtEnd - pointAtStart).length();
    Quat quat = Quat::from_axis_angle(localDir.cross(force).normalized(), sizeBranch * tenseForceFactor * 0.2f);

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
		if (isTrunk && float(detailParam.inBranchDepth+1) < param.trunkBranchRange.x())
			return node;

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
        int nbBranch = int(nbBranchf) + (_random(_randEngine) < fmodf(nbBranchf, 1) ? 1:0);
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
        int nbLeaf = int(nbLeaff) + (_random(_randEngine) < fmodf(nbLeaff, 1) ? 1:0);
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
    x = fmodf(x, 1);

    dir = interpolate(curve.computeDirection(ix), curve.computeDirection(ix2), x);
    thickness = interpolate(curve.radius(ix), curve.radius(ix2), x);
    return interpolate(curve.point(ix), curve.point(ix2), x);
}

namespace
{
	vec2 altereRange(vec2 range, float tr, float scale)
	{
		float med = (range[0] + range[1]) * 0.5f;
		return ((range - med) * scale) + med + tr;
	}

	float genAround(float center, float size, float amount, float randf)
	{
		return (center - size*0.5f*amount) + randf*size*0.5f*amount;
	}
}

LTree::Parameter& LTree::Parameter::alterate(int seed, float amount)
{
	amount = std::min(2.f, amount);
	std::mt19937 gen(seed);
	std::uniform_real_distribution<float> random(0, 1);
	std::uniform_int_distribution<int> random_int(0, 2);

	bool grow = gen() % 2 == 0;
	int sign = grow ? 1 : -1;

	nbTrunkStep += (static_cast<int>(random_int(gen)*sign*amount + 0.5f));
	nbTrunkStep = std::max(1, nbTrunkStep);
	
	trunkAngle.translate_scale(genAround(0, 8, amount, random(gen)), genAround(1, 1, amount, random(gen))).makePositive();

	trunkStepSize *= (1.f + random(gen) * amount * sign * 0.5f);
	trunkStepSizeDecay.translate_scale(genAround(0, 0.15f, amount, random(gen)), genAround(1, 0.2f, amount, random(gen))).makePositive();

	branchAngle.translate_scale(genAround(0, 30, amount, random(gen)), genAround(1, 0.8f, amount, random(gen))).makePositive();
	
	for (auto& c : branchSplitDensity)
		c *= genAround(1, 1.75, amount, random(gen));

	branchSplitNoise *= genAround(1, 1, amount, random(gen));

	initialBranchSize *= (1 + sign*amount*0.5f*random(gen));
	branchSizeDecay.translate_scale(genAround(0, 0.2f, amount, random(gen)), 1).makePositive();
	branchSizeCoef.translate_scale(0, genAround(1, 0.3f, amount, random(gen))).makePositive();

	trunkBranchRange[0] *= genAround(1, 0.5f, amount, random(gen));
	trunkBranchRange[0] = std::min(float(nbTrunkStep)-0.1f, trunkBranchRange[0]),
	trunkBranchDensity.translate_scale(genAround(0, 3, amount, random(gen)), genAround(1, 0.6f, amount, random(gen))).makePositive();
	extraBranchDensity.translate_scale(genAround(0, 3, amount, random(gen)), genAround(1, 0.6f, amount, random(gen))).makePositive();

	branchJitter.translate_scale(genAround(0, 0.2f, amount, random(gen)), genAround(1, 0.6f, amount, random(gen))).makePositive();
	naturalBranchBending.translate_scale(genAround(0, 0.3f, amount, random(gen)), genAround(1, 0.6f, amount, random(gen)));

	curvatureForce += vec3(genAround(0, 1, amount, random(gen)),
						   genAround(0, 1, amount, random(gen)),
		                   genAround(0, 1, amount, random(gen)));

	curvatureThicknessResistance += vec2(genAround(0, 0.2f, amount, random(gen)), genAround(0, 0.2f, amount, random(gen)));

	float thicknessCoef = genAround(1, 0.3f, amount, random(gen));
	meshing.initialBranchThickness *= thicknessCoef;
	meshing.trunkThickness *= thicknessCoef;

	return *this;
}


void LTree::Parameter::print() const
{
    std::cout << "nbTrunkStep=" << nbTrunkStep << std::endl;
	std::cout << "trunkAngle="; trunkAngle.print(std::cout) << std::endl;
	std::cout << "trunkStepSize="; trunkStepSize.print(std::cout) << std::endl;
	std::cout << "trunkStepSizeDecay="; trunkStepSizeDecay.print(std::cout) << std::endl;
    std::cout << "depth=" << depth << std::endl;
    std::cout << "curveResolution=" << curveResolution << std::endl;
	std::cout << "branchAngle=";  branchAngle.print(std::cout) << std::endl;
	std::cout << "firstBranchAngleCoef="; firstBranchAngleCoef.print(std::cout) << std::endl;
    std::cout << "branchSplitNoise=" << branchSplitNoise << std::endl;
    std::cout << "initialBranchSize=" << initialBranchSize << std::endl;
	std::cout << "branchSizeDecay=";  branchSizeDecay.print(std::cout) << std::endl;
	std::cout << "branchSizeCoef=";  branchSizeCoef.print(std::cout) << std::endl;
    std::cout << "branchSizeStopThreshold=" << branchSizeStopThreshold << std::endl;
	std::cout << "trunkBranchRange=" << trunkBranchRange << std::endl;
	std::cout << "trunkBranchDensity=";  trunkBranchDensity.print(std::cout) << std::endl;
	std::cout << "extraBranchDensity=";  extraBranchDensity.print(std::cout) << std::endl;
    std::cout << "extraBranchSpacing=" << extraBranchSpacing << std::endl;
	std::cout << "branchJitter="; branchJitter.print(std::cout) << std::endl;
	std::cout << "naturalBranchBending=";  naturalBranchBending.print(std::cout) << std::endl;
    std::cout << "curvatureForce=" << curvatureForce[2] << std::endl;
    std::cout << "curvatureThicknessResistance=" << curvatureThicknessResistance << std::endl;
}


LTree::MeshingParameter& LTree::MeshingParameter::scale(float x)
{
	this->initialBranchThickness *= x;
	this->trunkThickness *= x;
	return *this;
}

LTree::MeshingParameter& LTree::MeshingParameter::scaleDecay(tim::vec2 coef)
{
	this->alongBranchThicknessDecay *= coef.y();
	this->alongTrunkThicknessDecay *= coef.x();
	this->branchThicknessDecay *= coef.y();
	this->trunkThicknessDecay *= coef.x();
	return *this;
}

LTree::MeshingParameter LTree::MeshingParameter::interpolate(const MeshingParameter& m1, const MeshingParameter& m2, float coef)
{
	MeshingParameter param;
	param.alongBranchThicknessDecay = tim::interpolate(m1.alongBranchThicknessDecay, m2.alongBranchThicknessDecay, coef);
	param.alongTrunkThicknessDecay = tim::interpolate(m1.alongTrunkThicknessDecay, m2.alongTrunkThicknessDecay, coef);
	param.branchThicknessDecay = tim::interpolate(m1.branchThicknessDecay, m2.branchThicknessDecay, coef);
	param.initialBranchThickness = tim::interpolate(m1.initialBranchThickness, m2.initialBranchThickness, coef);
	param.trunkThickness = tim::interpolate(m1.trunkThickness, m2.trunkThickness, coef);
	param.trunkThicknessDecay = tim::interpolate(m1.trunkThicknessDecay, m2.trunkThicknessDecay, coef);
	return param;
}

eastl::vector<float> LTree::Parameter::interpolate(const eastl::vector<float>& v1, const eastl::vector<float>& v2, float coef)
{
	eastl::vector<float> v = v1.size() > v2.size() ? v1 : v2;
	for (size_t i = 0; i < std::min(v1.size(), v2.size()); ++i)
		v[i] = tim::interpolate(v1[i], v2[i], coef);

	return v;
}

LTree::Parameter LTree::Parameter::interpolate(const Parameter& p1, const Parameter& p2, float coef)
{
	Parameter p;
	p.branchAngle = GaussPDF::interpolate(p1.branchAngle, p2.branchAngle, coef);
	p.branchEarlyTermination = interpolate(p1.branchEarlyTermination, p2.branchEarlyTermination, coef);

	p.branchJitter = GaussPDF::interpolate(p1.branchJitter, p2.branchJitter, coef);
	p.branchSizeAlongTrunk = SampleFunction::interpolate(p1.branchSizeAlongTrunk, p2.branchSizeAlongTrunk, coef);
	p.branchSizeCoef = GaussPDF::interpolate(p1.branchSizeCoef, p2.branchSizeCoef, coef);
	p.branchSizeDecay = GaussPDF::interpolate(p1.branchSizeDecay, p2.branchSizeDecay, coef);
	p.branchSizeStopThreshold = tim::interpolate(p1.branchSizeStopThreshold, p2.branchSizeStopThreshold, coef);
	p.branchSplitDensity = interpolate(p1.branchSplitDensity, p2.branchSplitDensity, coef);
	p.branchSplitNoise = tim::interpolate(p1.branchSplitNoise, p2.branchSplitNoise, coef);
	
	p.curvatureForce = tim::interpolate(p1.curvatureForce, p2.curvatureForce, coef);
	p.curvatureThicknessResistance = tim::interpolate(p1.curvatureThicknessResistance, p2.curvatureThicknessResistance, coef);
	p.curveResolution = tim::interpolate(p1.curveResolution, p2.curveResolution, coef);
	p.depth = int(tim::interpolate(float(p1.depth), float(p2.depth), coef) + 0.5f);
	
	p.extraBranchDensity = GaussPDF::interpolate(p1.extraBranchDensity, p2.extraBranchDensity, coef);
	p.extraBranchSpacing = tim::interpolate(p1.extraBranchSpacing, p2.extraBranchSpacing, coef);
	p.firstBranchAngleCoef = GaussPDF::interpolate(p1.firstBranchAngleCoef, p2.firstBranchAngleCoef, coef);
	p.initialBranchSize = tim::interpolate(p1.initialBranchSize, p2.initialBranchSize, coef);
	p.meshing = MeshingParameter::interpolate(p1.meshing, p2.meshing, coef);
	p.naturalBranchBending = GaussPDF::interpolate(p1.naturalBranchBending, p2.naturalBranchBending, coef);
	p.nbTrunkStep = int(tim::interpolate(float(p1.nbTrunkStep), float(p2.nbTrunkStep), coef) + 0.5f);
	
	p.trunkAngle = GaussPDF::interpolate(p1.trunkAngle, p2.trunkAngle, coef);
	p.trunkBranchDensity = GaussPDF::interpolate(p1.trunkBranchDensity, p2.trunkBranchDensity, coef);
	p.trunkBranchRange = tim::interpolate(p1.trunkBranchRange, p2.trunkBranchRange, coef);
	p.trunkStepSize = GaussPDF::interpolate(p1.trunkStepSize, p2.trunkStepSize, coef);
	p.trunkStepSizeDecay = GaussPDF::interpolate(p1.trunkStepSizeDecay, p2.trunkStepSizeDecay, coef);

	return p;
}

LTree::LeafParameter LTree::LeafParameter::gen(int seed, int sizeCategorie)
{
	std::mt19937 gen(seed);
	std::uniform_real_distribution<float> random(0, 1);

	LeafParameter param;

	param.depth = gen() % 3;
	param.tilt = vec2(-PI / 3, PI / 3) * random(gen);
	param.orientation = vec2(-PI / 2, PI / 2) * (0.5f + random(gen)*0.5f);

	switch (sizeCategorie)
	{
	case 0:
		param.scale = std::uniform_real_distribution<float>(0.1, 0.4)(gen);
		param.density = std::uniform_real_distribution<float>(5, 10)(gen);
		break;

	case 1:
		param.scale = std::uniform_real_distribution<float>(0.4, 0.7)(gen);
		param.density = std::uniform_real_distribution<float>(3, 6)(gen);
		break;

	case 2:
		param.scale = std::uniform_real_distribution<float>(0.7, 1.5)(gen);
		param.density = std::uniform_real_distribution<float>(1,3)(gen);
		break;
	}

	return param;
}

}
