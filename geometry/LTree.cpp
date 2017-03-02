
#include "LTree.h"

namespace tim
{

LTree::LTree() : _randEngine(42), _random(0,1)
{
    _root = &_nodePool.push_back();

	Curve c;
	c.addPoint({ 0,0,0 });
	c.addPoint({ 0,0,2 });

	_root->curve = c;

	generateTree(_root, 0, 2);
}

void LTree::exportOBJ(eastl::string filename) const
{
    Mesh acc;
    accumulateMesh(acc, _root);

    acc.exportToObj(filename);
}

void LTree::accumulateMesh(Mesh& acc, const LTree::Node* node)
{
    acc += node->curve.convertToMesh();
    for(Node* n : node->nodes)
        accumulateMesh(acc, n);
}

void LTree::generateTree(Node* node, int depth, int maxDepth)
{
	if (depth >= maxDepth)
		return;

	if (node == nullptr || node->curve.numPoints() == 0)
		return;

	vec3 last = node->curve.point(node->curve.numPoints() - 1);
	vec3 dir = node->curve.computeDirection(node->curve.numPoints() - 1);

	vec3 d = mat3::RotationZ(_random(_randEngine)*2.f*PI) * mat3::RotationX(toRad(45.f)) * vec3(0, 0, 1);
	dir = mat3::changeBasis(dir) * d;

	Node* newN = new Node;
	node->nodes.push_back(newN);
	newN->curve.addPoint(last);
	newN->curve.addPoint(last + dir * powf(2.f, -float(depth)));

	generateTree(newN, depth+1, maxDepth);
}

}
