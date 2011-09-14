/*
 *  MetaNode.cpp
 *  aomdd
 *
 *  Created by William Lam on May 10, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#include "MetaNode.h"
#include <sstream>


namespace aomdd {

using namespace std;

MetaNode::ANDNode::ANDNode() :
    weight(1) {
}

MetaNode::ANDNode::ANDNode(double w, unsigned int nc, MetaNode **ch) :
    weight(w), numChildren(nc), children(ch) {
}

MetaNode::ANDNode::~ANDNode() {
    delete []children;
}

double MetaNode::ANDNode::Evaluate(const Assignment &a) {
    double ret = weight;
    for (unsigned int i = 0; i < numChildren; ++i) {
        ret *= children[i]->Evaluate(a);
        if (ret == 0)
            return ret;
    }
    return ret;
}

double MetaNode::ANDNode::GetWeight() const {
    return weight;
}

void MetaNode::ANDNode::SetWeight(double w) {
    weight = w;
}

void MetaNode::ANDNode::Save(ostream &out, string prefix) const {
    out << prefix << "and-id: " << this << endl;
    out << prefix << "weight: " << weight << endl;
    out << prefix << "children: ";
    for (unsigned int i = 0; i < numChildren; ++i) {
        out << " " << children[i];
    }
}

void MetaNode::ANDNode::RecursivePrint(ostream &out, string prefix) const {
    Save(out, prefix); out << endl;
    for (unsigned int i = 0; i < numChildren; ++i) {
        children[i]->RecursivePrint(out, prefix + "    ");
        out << endl;
    }
}

// ======================

MetaNode::MetaNode() : varID(-1), card(0), elimValueCached(false) {
}

MetaNode::MetaNode(const Scope &var, ANDNode **ch) :
    children(ch), elimValueCached(false) {
    // Scope must be over one variable
    assert(var.GetNumVars() == 1);
    varID = var.GetOrdering().front();
    card = var.GetVarCard(varID);

    // All assignments must be specified
    hashVal = hash_value(*this);
//    assert(var.GetCard() == children.size() || children.size() == 1);
}

MetaNode::MetaNode(int varidIn, int cardIn, ANDNode **ch) :
    varID(varidIn), card(cardIn), children(ch), elimValueCached(false) {
    // Scope must be over one variable
    // All assignments must be specified
    hashVal = hash_value(*this);
//    assert(card == children.size());
}

MetaNode::~MetaNode() {
    /*
    for (unsigned int i = 0; i < card; ++i) {
        delete children[i];
    }
    */
    delete []children;
}

double MetaNode::Normalize() {
    if (IsTerminal()) {
        return 1.0;
    }
    double normValue = 0;
    for (unsigned int i = 0; i < card; ++i) {
        normValue += children[i]->GetWeight();
    }
    for (unsigned int i = 0; i < card; ++i) {
        children[i]->SetWeight(children[i]->GetWeight() / normValue);
    }
    return normValue;
}

double MetaNode::Maximum(const Assignment &a) {
    int val;
    if (this == MetaNode::GetZero()) {
        return 0.0;
    }
    else if (this == MetaNode::GetOne()) {
        return 1.0;
    }
    else if (elimValueCached) {
        return cachedElimValue;
    }
    // evidence case
    else if ( (val = a.GetVal(varID)) != ERROR_VAL) {
        double temp = children[val]->GetWeight();
        unsigned int nc = children[val]->GetNumChildren();
        MetaNode **andCh = children[val]->GetChildren();
        for (unsigned int i = 0; i < nc; ++i) {
            temp *= andCh[i]->Maximum(a);
        }
        cachedElimValue = temp;
        elimValueCached = true;
    }
    // no evidence
    else {
        double maxVal = DOUBLE_MIN;
        for (unsigned int i = 0; i < card; ++i) {
            double temp = children[i]->GetWeight();
            unsigned int nc = children[i]->GetNumChildren();
            MetaNode **andCh = children[i]->GetChildren();
            for (unsigned int j = 0; j < nc; ++j) {
                temp *= andCh[j]->Maximum(a);
            }
            if (temp > maxVal) maxVal = temp;
        }
        cachedElimValue = maxVal;
        elimValueCached = true;
    }
    return cachedElimValue;
}

double MetaNode::Sum(const Assignment &a) {
    int val;
    if (this == MetaNode::GetZero()) {
        return 0.0;
    }
    else if (this == MetaNode::GetOne()) {
        return 1.0;
    }
    else if (elimValueCached) {
        return cachedElimValue;
    }
    else if ( (val = a.GetVal(varID)) != ERROR_VAL) {
        double temp = children[val]->GetWeight();
        unsigned int nc = children[val]->GetNumChildren();
        MetaNode **andCh = children[val]->GetChildren();
        for (unsigned int i = 0; i < nc; ++i) {
            temp *= andCh[i]->Sum(a);
        }
        cachedElimValue = temp;
        elimValueCached = true;
    }
    else {
        double total = 0.0;
        for (unsigned int i = 0; i < card; ++i) {
            double temp = children[i]->GetWeight();
            unsigned int nc = children[i]->GetNumChildren();
            MetaNode **andCh = children[i]->GetChildren();
            for (unsigned int j = 0; j < nc; ++j) {
                temp *= andCh[j]->Sum(a);
            }
            total += temp;
        }
        cachedElimValue = total;
        elimValueCached = true;
    }
    return cachedElimValue;
}

double MetaNode::Evaluate(const Assignment &a) const {
    if (this == GetZero()) {
        return 0;
    }
    else if (this == GetOne()) {
        return 1;
    }
    // Handle dummy variable case
    else if (card == 1) {
        return children[0]->Evaluate(a);
    }
    else {
        int idx = a.GetVal(varID);
        assert(idx >= 0);
        return children[idx]->Evaluate(a);
    }
}

void MetaNode::Save(ostream &out, string prefix) const {
    if(this == MetaNode::GetZero()) {
        out << prefix << "TERMINAL ZERO" << endl;
    }
    else if(this == MetaNode::GetOne()) {
        out << prefix << "TERMINAL ONE" << endl;
    }
    else {
        out << prefix << "varID: " << varID;
    }
    out << endl;
    out << prefix << "id: " << this << endl;
//    out << prefix << "weight: " << weight << endl;
    out << prefix << "children: ";
    for (unsigned int i = 0; i < card; ++i) {
        out << " " << children[i];
    }
    if (card == 0) {
        out << "None";
    }
}

void MetaNode::RecursivePrint(ostream &out, string prefix) const {
    Save(out, prefix); out << endl;
    for (unsigned int i = 0; i < card; ++i) {
        children[i]->RecursivePrint(out, prefix + "    ");
        out << endl;
    }
}

void MetaNode::RecursivePrint(ostream &out) const {
    RecursivePrint(out, "");
}

pair<unsigned int, unsigned int> MetaNode::NumOfNodes() const {
    boost::unordered_set<const MetaNode *> nodeSet;
    FindUniqueNodes(nodeSet);
    unsigned int count = 0;
    BOOST_FOREACH(const MetaNode *m, nodeSet) {
        count += m->GetCard();
    }
    return pair<unsigned int, unsigned int>(nodeSet.size(), count);
}

void MetaNode::GetNumNodesPerVar(vector<unsigned int> &numMeta) const {
    boost::unordered_set<const MetaNode *> nodeSet;
    FindUniqueNodes(nodeSet);
    BOOST_FOREACH(const MetaNode *m, nodeSet) {
        int vid = m->GetVarID();
        assert(vid < int(numMeta.size()));
        numMeta[vid]++;
    }
}

double MetaNode::ComputeTotalMemory() const {
    unordered_set<const MetaNode *> nodeSet;
    FindUniqueNodes(nodeSet);
    double memUsage = 0;
    BOOST_FOREACH(const MetaNode *m, nodeSet) {
        memUsage += m->MemUsage();
    }
    memUsage /= pow(2.0, 20);
    return memUsage;
}

void MetaNode::FindUniqueNodes(boost::unordered_set<const MetaNode *> &nodeSet) const {
    if (IsTerminal()) return;
    unsigned int oldSize = nodeSet.size();
    nodeSet.insert(this);

    // Check if this has already been visited
    if (nodeSet.size() == oldSize) {
//        cout << "Found something isomorphic! (at var " << varID << ")" << endl;
        return;
    }
    for (unsigned int i = 0; i < card; ++i) {
        unsigned int nc = children[i]->GetNumChildren();
        MetaNode **andCh = children[i]->GetChildren();
        for (unsigned int j = 0; j < nc; ++j) {
            andCh[j]->FindUniqueNodes(nodeSet);
        }
    }
}

size_t hash_value(const MetaNode &node) {
    size_t seed = 0;
    boost::hash_combine(seed, node.varID);
    boost::hash_combine(seed, node.card);
    for (unsigned int i = 0; i < node.card; ++i) {
        boost::hash_combine(seed, node.children[i]->GetWeight());
        unsigned int nc = node.children[i]->GetNumChildren();
        for (unsigned int j = 0; j < nc; ++j) {
            boost::hash_combine(seed, node.children[i]->GetChildren()[j]);
        }
    }
    return seed;

}


// Initialize static variables
MetaNode *MetaNode::terminalZero;
MetaNode *MetaNode::terminalOne;
bool MetaNode::zeroInit = false;
bool MetaNode::oneInit = false;

} // end of aomdd namespace
