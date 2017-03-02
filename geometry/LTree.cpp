
#include "LTree.h"

namespace tim
{

LTree::LTree()
{
    _root = &_nodePool.push_back();
    _root->curve = Curve::parametrization({0,3}, 10, [](float){ return 0.f; }, [](float){ return 0.f; }, [](float x){ return x; });

    Node* newN = &_nodePool.push_back();
    newN->curve = Curve::parametrization({0,3}, 20, [](float x){ return x*3.f-1.f; }, [](float){ return 0.f; }, [](float){ return 1.f; });
    _root->nodes.push_back(newN);
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

}
