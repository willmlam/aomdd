#ifndef AOCMGRAPH_H_
#define AOCMGRAPH_H_

#include "base.h"
#include "graphbase.h"
#include "PseudoTree.h"

namespace aomdd {
    // Class to provide an interface to a loose representation of an AO graph
    // Also generates an AOGraph for output to dot, 
    // enforcing context-minimality
    class AOCMGraph {
        class AOCMGraphNode {
            private:
                int var;
                Assignment a;
                NodeType type;
                double weight;
            public:
                AOCMGraphNode();
                AOCMGraphNode(int _var, const Assignment &_a, NodeType _type, double _weight) : var(_var), a(_a), type(_type), weight(_weight) {}
                const Assignment &GetAssignment() const { return a; }
                NodeType GetType() const { return type; }
                double GetWeight() const { return weight; }
        };

        Model *m;
        PseudoTree *pt;
        AOCMGraphNode *root;
        google::sparse_hash_set<size_t> *nodes;
        public:
            AOCMGraph();
            AOCMGraph(Model *_m, PseudoTree *_pt) : m(_m), pt(_pt) {}
            double GetVal(const Assignment &a) const;
            void GenerateDot(std::ostream &out) const;
        private:
            void GenerateGraph();
    };
}

#endif
