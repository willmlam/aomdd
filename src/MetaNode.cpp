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

MetaNode::ANDNode::ANDNode(double w, const vector<MetaNodePtr> &ch) :
    weight(w), children(ch) {
}

MetaNode::ANDNode::~ANDNode() {
}

double MetaNode::ANDNode::Evaluate(const Assignment &a) {
    double ret = weight;
    BOOST_FOREACH(const MetaNodePtr &i, children) {
        ret *= i->Evaluate(a);
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

void MetaNode::ANDNode::SetChildren(const std::vector<MetaNodePtr> &ch) {
    children = ch;
}

const vector<MetaNodePtr> &MetaNode::ANDNode::GetChildren() const {
    return children;
}

void MetaNode::ANDNode::Save(ostream &out, string prefix) const {
    out << prefix << "and-id: " << this << endl;
    out << prefix << "weight: " << weight << endl;
    out << prefix << "children: ";
    BOOST_FOREACH(const MetaNodePtr &i, children) {
        out << " " << i;
    }
}

void MetaNode::ANDNode::RecursivePrint(ostream &out, string prefix) const {
    Save(out, prefix); out << endl;
    BOOST_FOREACH(const MetaNodePtr &i, children) {
        i->RecursivePrint(out, prefix + "    ");
        out << endl;
    }
}

void MetaNode::ANDNode::GenerateDiagram(DirectedGraph &diagram, const DVertexDesc &parent) const {
    stringstream ss;
    ss << this;
    string cur = ss.str();
    ss.clear();
    DVertexDesc current = add_vertex(cur, diagram);
    add_edge(parent, current, diagram);
    BOOST_FOREACH(const MetaNodePtr &i, children) {
        i->GenerateDiagram(diagram, current);
    }
}

bool operator==(const ANDNodePtr &lhs, const ANDNodePtr &rhs) {
    if (fabs(lhs->GetWeight() - rhs->GetWeight()) >= TOLERANCE || lhs->GetChildren().size()
            != rhs->GetChildren().size()) {
        return false;
    }
    for (unsigned int i = 0; i < lhs->GetChildren().size(); i++) {
        if (lhs->GetChildren()[i].get() != rhs->GetChildren()[i].get()) {
            return false;
        }
    }
    return true;
}

bool operator!=(const ANDNodePtr &lhs, const ANDNodePtr &rhs) {
    return !(lhs == rhs);
}

// ======================

MetaNode::MetaNode() : varID(-1), card(0), elimValueCached(false) {
}

MetaNode::MetaNode(const Scope &var, const vector<ANDNodePtr> &ch) :
    children(ch), elimValueCached(false) {
    // Scope must be over one variable
    assert(var.GetNumVars() == 1);
    varID = var.GetOrdering().front();
    card = var.GetVarCard(varID);

    // All assignments must be specified
    hashVal = hash_value(*this);
    assert(var.GetCard() == children.size() || children.size() == 1);
}

MetaNode::MetaNode(int varidIn, int cardIn, const vector<ANDNodePtr> &ch) :
    varID(varidIn), card(cardIn), children(ch), elimValueCached(false) {
    // Scope must be over one variable
    // All assignments must be specified
    hashVal = hash_value(*this);
    assert(card == children.size());
}

MetaNode::~MetaNode() {
}

double MetaNode::Normalize() {
    if (IsTerminal()) {
        return 1.0;
    }
    double normValue = 0;
    BOOST_FOREACH(ANDNodePtr &i, children) {
        normValue += i->GetWeight();
    }
    BOOST_FOREACH(ANDNodePtr &i, children) {
        i->SetWeight(i->GetWeight() / normValue);
    }
    return normValue;
}

double MetaNode::Maximum(const Assignment &a) {
    int val;
    if (this == MetaNode::GetZero().get()) {
        return 0.0;
    }
    else if (this == MetaNode::GetOne().get()) {
        return 1.0;
    }
    else if (elimValueCached) {
        return cachedElimValue;
    }
    else if ( (val = a.GetVal(varID)) != ERROR_VAL) {
        double temp = children[val]->GetWeight();
        BOOST_FOREACH(MetaNodePtr j, children[val]->GetChildren()) {
            temp *= j->Maximum(a);
        }
        cachedElimValue = temp;
        elimValueCached = true;
    }
    else {
        double maxVal = DOUBLE_MIN;
        BOOST_FOREACH(ANDNodePtr i, children) {
            double temp = i->GetWeight();
            BOOST_FOREACH(MetaNodePtr j, i->GetChildren()) {
                temp *= j->Maximum(a);
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
    if (this == MetaNode::GetZero().get()) {
        return 0.0;
    }
    else if (this == MetaNode::GetOne().get()) {
        return 1.0;
    }
    else if (elimValueCached) {
        return cachedElimValue;
    }
    else if ( (val = a.GetVal(varID)) != ERROR_VAL) {
        double temp = children[val]->GetWeight();
        BOOST_FOREACH(MetaNodePtr j, children[val]->GetChildren()) {
            temp *= j->Sum(a);
        }
        cachedElimValue = temp;
        elimValueCached = true;
    }
    else {
        double total = 0.0;
        BOOST_FOREACH(ANDNodePtr i, children) {
            double temp = i->GetWeight();
            BOOST_FOREACH(MetaNodePtr j, i->GetChildren()) {
                temp *= j->Sum(a);
            }
            total += temp;
        }
        cachedElimValue = total;
        elimValueCached = true;
    }
    return cachedElimValue;
}

double MetaNode::Evaluate(const Assignment &a) const {
    if (this == GetZero().get()) {
        return 0;
    }
    else if (this == GetOne().get()) {
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
    if(this == MetaNode::GetZero().get()) {
        out << prefix << "TERMINAL ZERO" << endl;
    }
    else if(this == MetaNode::GetOne().get()) {
        out << prefix << "TERMINAL ONE" << endl;
    }
    else {
        out << prefix << "varID: " << varID;
        if (IsDummy()) {
            out << " (dummy)";
        }
    }
    out << endl;
    out << prefix << "id: " << this << endl;
//    out << prefix << "weight: " << weight << endl;
    out << prefix << "children: ";
    BOOST_FOREACH(const ANDNodePtr &i, children)
                {
                    out << " " << i;
                }
    if (children.empty()) {
        out << "None";
    }
}

void MetaNode::RecursivePrint(ostream &out, string prefix) const {
    Save(out, prefix); out << endl;
    BOOST_FOREACH(const ANDNodePtr &i, children) {
        i->RecursivePrint(out, prefix + "    ");
        out << endl;
    }
}

void MetaNode::RecursivePrint(ostream &out) const {
    RecursivePrint(out, "");
}

DirectedGraph MetaNode::GenerateDiagram() const {
    DirectedGraph diagram;
    DVertexDesc dummy(VertexDesc(0));
    GenerateDiagram(diagram, dummy);
    return diagram;
}

void MetaNode::GenerateDiagram(DirectedGraph &diagram, const DVertexDesc &parent) const {
    stringstream ss;
    ss << this;
    string cur = ss.str();
    ss.clear();
    DVertexDesc current = add_vertex(cur, diagram);
    add_edge(parent, current, diagram);
    BOOST_FOREACH(const ANDNodePtr &i, children) {
        i->GenerateDiagram(diagram, current);
    }
}

int MetaNode::NumOfNodes() const {
    boost::unordered_set<size_t> nodeSet;
    NumOfNodes(nodeSet);
    return nodeSet.size();
}

void MetaNode::NumOfNodes(boost::unordered_set<size_t> &nodeSet) const {
    if (IsTerminal()) return;
    unsigned int oldSize = nodeSet.size();
    nodeSet.insert(size_t(this));

    // Check if this has already been visited
    if (nodeSet.size() == oldSize) {
//        cout << "Found something isomorphic! (at var " << varID << ")" << endl;
        return;
    }
    BOOST_FOREACH(const ANDNodePtr &i, children) {
        BOOST_FOREACH(const MetaNodePtr &j, i->GetChildren()) {
            j->NumOfNodes(nodeSet);
        }
    }
}

size_t hash_value(const MetaNode &node) {
    size_t seed = 0;
    boost::hash_combine(seed, node.varID);
    boost::hash_combine(seed, node.card);
    BOOST_FOREACH(const ANDNodePtr &i, node.children) {
        boost::hash_combine(seed, i->GetWeight());
        BOOST_FOREACH(const MetaNodePtr &j, i->GetChildren()) {
            boost::hash_combine(seed, j.get());
        }
    }
    return seed;

}

size_t hash_value(const MetaNodePtr &node) {
    size_t seed = 0;
    boost::hash_combine(seed, node->GetVarID());
    boost::hash_combine(seed, node->GetCard());
    BOOST_FOREACH(const ANDNodePtr &i, node->GetChildren()) {
        boost::hash_combine(seed, i->GetWeight());
        BOOST_FOREACH(const MetaNodePtr &j, i->GetChildren()) {
            boost::hash_combine(seed, j.get());
        }
    }
    return seed;
}

bool operator==(const MetaNodePtr &lhs, const MetaNodePtr &rhs) {
    if (lhs.get() == MetaNode::GetZero().get() && rhs.get()
            == MetaNode::GetOne().get())
        return false;
    if (rhs.get() == MetaNode::GetZero().get() && lhs.get()
            == MetaNode::GetOne().get())
        return false;
    if (lhs->GetStoredHash() != rhs->GetStoredHash()) {
        return false;
    }
    if (lhs->GetVarID() != rhs->GetVarID() || lhs->GetCard() != rhs->GetCard()
            || lhs->GetChildren().size() != rhs->GetChildren().size())
        return false;
    for (unsigned int i = 0; i < lhs->GetChildren().size(); i++) {
        if (lhs->GetChildren()[i] != rhs->GetChildren()[i])
            return false;
    }
    return true;
}

bool operator!=(const MetaNodePtr &lhs, const MetaNodePtr &rhs) {
    return !(lhs == rhs);
}


// Initialize static variables
MetaNodePtr MetaNode::terminalZero;
MetaNodePtr MetaNode::terminalOne;
bool MetaNode::zeroInit = false;
bool MetaNode::oneInit = false;

} // end of aomdd namespace
