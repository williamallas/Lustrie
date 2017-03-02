#pragma once

#include "Curve.h"
#include <EASTL/deque.h>

namespace tim
{
    class LTree
    {
    public:
        LTree();
        ~LTree() = default;

        LTree(const LTree&) = default;
        LTree(LTree&&) = default;

        LTree& operator=(const LTree&) = default;
        LTree& operator=(LTree&&) = default;

        void exportOBJ(eastl::string) const;

    private:
        struct Node
        {
            Curve curve;
            eastl::vector<Node*> nodes;
        };
        Node* _root = nullptr;

        eastl::deque<Node, EASTLAllocatorType, 256> _nodePool;

    private:
        static void accumulateMesh(Mesh&, const Node*);

    };
}
