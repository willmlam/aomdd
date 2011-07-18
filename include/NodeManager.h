/*
 *  NodeManager.h
 *  aomdd
 *
 *  Created by William Lam on May 11, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#ifndef NODEMANAGER_H_
#define NODEMANAGER_H_

#include "base.h"
#include "graphbase.h"
#include "MetaNode.h"

namespace aomdd {

enum Operator {
    PROD, SUM, REDUCE, MARGINALIZE, NONE
};

// Note: set comparison will use overridden version of == for MetaNodePtr
// Should be ok...

typedef boost::unordered_set<MetaNodePtr> ParamSet;

class Operation {
    Operator op;
    ParamSet params;
    MetaNodePtr result;

public:
    Operation() :
        op(NONE) {
    }
    Operation(Operator o, MetaNodePtr arg) :
        op(o) {
        params.insert(arg);
    }
    Operation(Operator o, MetaNodePtr arg1, const std::vector<MetaNodePtr> &arg2) :
        op(o) {
        params.insert(arg1);
        for (unsigned int i = 0; i < arg2.size(); ++i) {
            params.insert(arg2[i]);
        }
    }

    const Operator &GetOperator() const {
        return op;
    }

    const ParamSet &GetParamSet() const {
        return params;
    }
};

size_t hash_value(const Operation &o);
bool operator==(const Operation &lhs, const Operation &rhs);

typedef boost::unordered_set<MetaNodePtr> UniqueTable;
typedef boost::unordered_map<Operation, MetaNodePtr> OperationCache;
typedef std::pair<MetaNodePtr, std::vector<MetaNodePtr> > ApplyParamSet;

class NodeManager {
    UniqueTable ut;
    OperationCache opCache;
    NodeManager() {
    }
    NodeManager(NodeManager const&) {
    }
    NodeManager& operator=(NodeManager const&) {
        return *this;
    }

    static bool initialized;
    static NodeManager *singleton;

    // Reweigh nodes by multiplying in w to the MetaNode, unless it's a terminal
    std::vector<MetaNodePtr> ReweighNodes(
            const std::vector<MetaNodePtr> &nodes, double w);
    MetaNodePtr NormalizeHelper(MetaNodePtr root);

    std::vector<MetaNodePtr> CopyMetaNodes(
            const std::vector<MetaNodePtr> &nodes);
    std::vector<ANDNodePtr> CopyANDNodes(
            const std::vector<ANDNodePtr> &nodes);

public:
    static NodeManager *GetNodeManager();
    // Create a metanode from a variable with a children list
    MetaNodePtr CreateMetaNode(const Scope &var,
            const std::vector<ANDNodePtr> &ch, double weight = 1);

    MetaNodePtr CreateMetaNode(int varid, unsigned int card,
            const std::vector<ANDNodePtr> &ch, double weight = 1);

    // Create a metanode based on a tabular form of the function
    // Variable ordering is defined by the scope
    MetaNodePtr CreateMetaNode(const Scope &vars,
            const std::vector<double> &vals, double weight = 1);

    // Be sure the input node is a root!
    // Returns a vector of pointers since ANDNodes can have multiple
    // MetaNode children
    std::vector<MetaNodePtr> FullReduce(MetaNodePtr node, double &w);

    // Driver function that returns a dummy root if needed (to combine
    // multiple MetaNodes as a single output
    MetaNodePtr FullReduce(MetaNodePtr node);


    MetaNodePtr Apply(MetaNodePtr lhs, const std::vector<MetaNodePtr> &rhs, Operator op,
            const DirectedGraph &embeddedPT, double w = 1);

    std::vector<ApplyParamSet> GetParamSets(const DirectedGraph &tree,
            const std::vector<MetaNodePtr> &lhs,
            const std::vector<MetaNodePtr> &rhs) const;

    MetaNodePtr Marginalize(MetaNodePtr root, const Scope &s, const DirectedGraph &embeddedPT);
    MetaNodePtr Condition(MetaNodePtr root, const Assignment &cond);
    MetaNodePtr Normalize(MetaNodePtr root);


    unsigned int GetNumberOfNodes() const;

    void PrintUniqueTable(std::ostream &out) const;
    void PrintReferenceCount(std::ostream &out) const;

};

} // end of aomdd namespace

#endif /* NODEMANAGER_H_ */
