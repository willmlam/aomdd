/*
 *  MetaNode.h
 *  aomdd
 *
 *  Created by William Lam on 3/23/11.
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

// Note these classes do not own the pointers themselves, the node manager does.

#ifndef _METANODE_H_
#define _METANODE_H_

#include "base.h"
#include "Scope.h"

namespace aomdd {

class MetaNode {

public:
    typedef boost::shared_ptr<MetaNode> MetaNodePtr;

    // Used to represent children of the metanode
    // Corresponds to weights of assignments
    class ANDNode {
        double weight;
        std::vector<MetaNodePtr> children;
    public:
        ANDNode();
        virtual ~ANDNode();

        ANDNode(double w, const std::vector<MetaNodePtr> &ch);

        double GetWeight() const;

        const std::vector<MetaNodePtr> &GetChildren() const;

        double Evaluate(const Assignment &a);

        bool operator==(const ANDNode &rhs) const;

        void Save(std::ostream &out);
    };

    typedef boost::shared_ptr<ANDNode> ANDNodePtr;

private:
    // IDs handled by shared_ptr wrappers
    // Scope should contain only one variable, or if scope is empty scope,
    // this node is a terminal
    // Pointer due to plan to use a set of common Scope objects for all nodes
    // of the same variable
    int varID;
    unsigned int card;
    // children: ANDNodes
    std::vector<ANDNodePtr> children;

    // Used to make terminal nodes singletons
    static bool zeroInit;
    static bool oneInit;
    static MetaNodePtr terminalZero;
    static MetaNodePtr terminalOne;

    static unsigned int idCount;

public:
    MetaNode();

    virtual ~MetaNode();

    MetaNode(const Scope &var, const std::vector<ANDNodePtr> &ch);

    int GetVarID() const {
        return varID;
    }

    unsigned int GetCard() const {
        return card;
    }

    double Evaluate(const Assignment &a) const;

    const std::vector<ANDNodePtr> &GetChildren() const;

    bool operator==(const MetaNode &rhs) const;
    void Save(std::ostream &out);

    friend size_t hash_value(const MetaNode &node);

    static const MetaNodePtr &GetZero();
    static const MetaNodePtr &GetOne();

};

typedef MetaNode::MetaNodePtr MetaNodePtr;
typedef MetaNode::ANDNodePtr ANDNodePtr;

bool operator==(const MetaNodePtr &lhs, const MetaNodePtr &rhs);

} // end of aomdd namespace

#endif
