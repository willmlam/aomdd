/*
 *  DDNode.h
 *  aomdd
 *
 *  Created by William Lam on 3/23/11.
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#ifndef _METANODE_H_
#define _METANODE_H_

#include "base.h"
#include "Scope.h"

namespace aomdd {

class MetaNode {

    // Used to represent children of the metanode
    // Corresponds to weights of assignments
public:
    class ANDNode {
        double weight;
        std::vector<MetaNode*> children;
    public:
        ANDNode();

        ANDNode(double w, const std::vector<MetaNode*> &ch);

        double Evaluate(const Assignment &a);

        void Save(std::ostream &out);
    };

private:
    // ID: by pointer value
    // Scope should contain only one variable, or if scope is empty scope,
    // this node is a terminal
    // Leaving this open for flexibility (Nodes over multiple vars, etc.)
    Scope s;

    // Weight of this node
//    double weight;

    // children: ANDNodes
    std::vector<ANDNode*> children;


    // Used to make terminal nodes singletons
    static bool zeroInit;
    static bool oneInit;
    static MetaNode* terminalZero;
    static MetaNode* terminalOne;

public:
    MetaNode();

    MetaNode(const Scope &var, const std::vector<ANDNode*> &ch);

    double Evaluate(const Assignment &a) const;
    void Save(std::ostream &out);

    static MetaNode* GetZero();
    static MetaNode* GetOne();

};


} // end of aomdd namespace

#endif
