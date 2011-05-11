/*
 * MetaNode.cpp
 * aomdd
 *
 *  Created by William Lam on May 10, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#include "MetaNode.h"

namespace aomdd {
using namespace std;


MetaNode::ANDNode::ANDNode() :
    weight(0) {
}

MetaNode::ANDNode::ANDNode(double w, const vector<MetaNode*> &ch) :
    weight(w), children(ch) {
}

double MetaNode::ANDNode::Evaluate(const Assignment &a) {
    double ret = weight;
    BOOST_FOREACH(MetaNode *i, children) {
        ret *= i->Evaluate(a);
        if(ret == 0) return ret;
    }
    return ret;
}

void MetaNode::ANDNode::Save(ostream &out) {
    out << "id: " << this << endl;
    out << "weight: " << weight << endl;
    out << "children: ";
    BOOST_FOREACH(MetaNode *i, children) {
        out << " " << i;
    }
}


// ======================

MetaNode::MetaNode() {
}


MetaNode::MetaNode(const Scope &var, const vector<ANDNode*> &ch) :
    s(var), children(ch) {
    // All assignments must be specified
    assert(s.GetNumVars() == 1);
    assert(var.GetCard() == children.size());
}

double MetaNode::Evaluate(const Assignment &a) const {
    if(this == GetZero()) {
        return 0;
    }
    else if(this == GetOne()) {
        return 1;
    }
    else {
        unsigned int idx = a.GetVal(s.GetOrdering().front());
        return children[idx]->Evaluate(a);
    }
}

void MetaNode::Save(ostream &out) {
    out << "id: " << this << endl;
    out << "children: ";
    BOOST_FOREACH(ANDNode *i, children) {
        out << " " << i;
    }
    if(children.empty())
        out << "None";
}

MetaNode* MetaNode::GetZero() {
    if(!zeroInit) {
        terminalZero = new MetaNode();
        zeroInit = true;
        return terminalZero;
    }
    else {
        return terminalZero;
    }
}

MetaNode* MetaNode::GetOne() {
    if(!oneInit) {
        terminalOne = new MetaNode();
        oneInit = true;
        return terminalOne;
    }
    else {
        return terminalOne;
    }
}

// Initialize static variables
MetaNode* MetaNode::terminalZero = NULL;
MetaNode* MetaNode::terminalOne = NULL;
bool MetaNode::zeroInit = false;
bool MetaNode::oneInit = false;

} // end of aomdd namespace
