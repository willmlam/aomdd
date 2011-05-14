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
#include "MetaNode.h"

namespace aomdd {

enum Operator {
    PROD, SUM, REDUCE, NONE
};

// Note: set comparison will use overridden version of == for MetaNodePtr
// Should be ok...

typedef boost::unordered_set<MetaNodePtr> ParamSet;

class Operation {
    Operator op;
    ParamSet params;
public:
    Operation() :
        op(NONE) {
    }
    Operation(Operator o, MetaNodePtr arg) :
        op(o) {
        params.insert(arg);
    }
    Operation(Operator o, MetaNodePtr arg1, MetaNodePtr arg2) :
        op(o) {
        params.insert(arg1);
        params.insert(arg2);
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
public:
    static NodeManager *GetNodeManager();
    // Create a metanode from a variable with a children list
    MetaNodePtr CreateMetaNode(const Scope &var,
            const std::vector<ANDNodePtr> &ch);

    // Create a metanode based on a tabular form of the function
    // Variable ordering is defined by the scope
    MetaNodePtr CreateMetaNode(const Scope &vars,
            const std::vector<double> &vals);

    unsigned int GetNumberOfNodes() const;


};

} // end of aomdd namespace

#endif /* NODEMANAGER_H_ */
