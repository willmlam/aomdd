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
    weight(0) {
}

MetaNode::ANDNode::ANDNode(double w, const vector<MetaNodePtr> &ch) :
    weight(w), children(ch) {
}

MetaNode::ANDNode::~ANDNode() {
}

double MetaNode::ANDNode::Normalize() {
    BOOST_FOREACH(MetaNodePtr i, children) {
        weight *= i->Normalize();
        i->SetWeight(1);
    }
    return weight;
}

double MetaNode::ANDNode::Evaluate(const Assignment &a) {
    double ret = weight;
    BOOST_FOREACH(MetaNodePtr i, children)
                {
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

const vector<MetaNodePtr> &MetaNode::ANDNode::GetChildren() const {
    return children;
}


bool MetaNode::ANDNode::operator==(const ANDNode &rhs) const {
    if (weight != rhs.weight || children.size() != rhs.children.size()) {
        return false;
    }
    for (unsigned int i = 0; i < children.size(); i++) {
        if (children[i].get() != rhs.children[i].get()) {
            return false;
        }
    }
    return true;
}

void MetaNode::ANDNode::Save(ostream &out) {
    out << "and-id: " << this << endl;
    out << "weight: " << weight << endl;
    out << "children: ";
    BOOST_FOREACH(MetaNodePtr i, children)
                {
                    out << " " << i;
                }
}

void MetaNode::ANDNode::RecursivePrint(ostream &out) {
    Save(out); out << endl;
    BOOST_FOREACH(MetaNodePtr i, children) {
        i->RecursivePrint(out);
    }
}

void MetaNode::ANDNode::GenerateDiagram(DirectedGraph &diagram, const DVertexDesc &parent) const {
    stringstream ss;
    ss << this;
    string cur = ss.str();
    ss.clear();
    DVertexDesc current = add_vertex(cur, diagram);
    add_edge(parent, current, diagram);
    BOOST_FOREACH(MetaNodePtr i, children) {
        i->GenerateDiagram(diagram, current);
    }
}

bool operator==(const ANDNodePtr &lhs, const ANDNodePtr &rhs) {
    if (lhs->GetWeight() != rhs->GetWeight() || lhs->GetChildren().size()
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
    if (lhs->GetWeight() != rhs->GetWeight() || lhs->GetChildren().size()
            != rhs->GetChildren().size()) {
        return true;
    }
    for (unsigned int i = 0; i < lhs->GetChildren().size(); i++) {
        if (lhs->GetChildren()[i].get() != rhs->GetChildren()[i].get()) {
            return true;
        }
    }
    return false;

}

// ======================

MetaNode::MetaNode() : varID(-1), card(0), weight(1) {
}

MetaNode::MetaNode(const Scope &var, const vector<ANDNodePtr> &ch) :
    children(ch) {
    // Scope must be over one variable
    assert(var.GetNumVars() == 1);
    varID = var.GetOrdering().front();
    card = var.GetVarCard(varID);
    // All assignments must be specified
    assert(var.GetCard() == children.size());
}

MetaNode::~MetaNode() {
}



double MetaNode::Normalize() {
    double normConstant;
    BOOST_FOREACH(ANDNodePtr i, children) {
        normConstant += i->Normalize();
    }
    BOOST_FOREACH(ANDNodePtr i, children) {
        i->SetWeight(i->GetWeight() / normConstant);
    }
    return weight = normConstant;
}

double MetaNode::Evaluate(const Assignment &a) const {
    if (this == GetZero().get()) {
        return 0;
    }
    else if (this == GetOne().get()) {
        return 1;
    }
    else {
        unsigned int idx = a.GetVal(varID);
        return weight * children[idx]->Evaluate(a);
    }
}

// Considered equal if the scope and children pointers are the same
bool MetaNode::operator==(const MetaNode &rhs) const {
    if (this == GetZero().get() || this == GetOne().get())
        return false;
    if (varID != rhs.varID || card != rhs.card || children.size()
            != rhs.children.size())
        return false;
    for (unsigned int i = 0; i < children.size(); i++) {
        if (children[i] != rhs.children[i])
            return false;
    }
    return true;
}

void MetaNode::Save(ostream &out) {
    if(this == MetaNode::GetZero().get()) {
        out << "TERMINAL ZERO" << endl;
    }
    else if(this == MetaNode::GetOne().get()) {
        out << "TERMINAL ONE" << endl;
    }
    else {
        out << "varID: " << varID << endl;
    }
    out << "id: " << this << endl;
    out << "children: ";
    BOOST_FOREACH(ANDNodePtr i, children)
                {
                    out << " " << i;
                }
    if (children.empty()) {
        out << "None";
    }
}

void MetaNode::RecursivePrint(ostream &out) {
    Save(out); out << endl;
    BOOST_FOREACH(ANDNodePtr i, children) {
        i->RecursivePrint(out);
    }
    out << endl;
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
    BOOST_FOREACH(ANDNodePtr i, children) {
        i->GenerateDiagram(diagram, current);
    }
}

const MetaNodePtr &MetaNode::GetZero() {
    if (!zeroInit) {
        terminalZero = MetaNodePtr(new MetaNode());
        zeroInit = true;
        return terminalZero;
    }
    else {
        return terminalZero;
    }
}

const MetaNodePtr &MetaNode::GetOne() {
    if (!oneInit) {
        terminalOne = MetaNodePtr(new MetaNode());
        oneInit = true;
        return terminalOne;
    }
    else {
        return terminalOne;
    }
}

size_t hash_value(const MetaNode &node) {
    size_t seed = 0;
    boost::hash_combine(seed, node.varID);
    boost::hash_combine(seed, node.card);
    boost::hash_combine(seed, node.weight);
    BOOST_FOREACH(ANDNodePtr i, node.children) {
        boost::hash_combine(seed, i.get());
        BOOST_FOREACH(MetaNodePtr j, i->GetChildren()) {
            boost::hash_combine(seed, j.get());
        }
    }
    return seed;

}

size_t hash_value(const MetaNodePtr &node) {
    size_t seed = 0;
    boost::hash_combine(seed, node->GetVarID());
    boost::hash_combine(seed, node->GetCard());
    boost::hash_combine(seed, node->GetWeight());
    BOOST_FOREACH(ANDNodePtr i, node->GetChildren()) {
        boost::hash_combine(seed, i->GetWeight());
        BOOST_FOREACH(MetaNodePtr j, i->GetChildren()) {
            boost::hash_combine(seed, j.get());
        }
    }
    return seed;
}

bool operator==(const MetaNodePtr &lhs, const MetaNodePtr &rhs) {
//    cout << "Equality Checker Called" << endl;
    if (lhs.get() == MetaNode::GetZero().get() && rhs.get()
            == MetaNode::GetOne().get())
        return false;
    if (rhs.get() == MetaNode::GetZero().get() && lhs.get()
            == MetaNode::GetOne().get())
        return false;
    if (lhs->GetVarID() != rhs->GetVarID() || lhs->GetCard() != rhs->GetCard()
            || lhs->GetChildren().size() != rhs->GetChildren().size())
        return false;
    for (unsigned int i = 0; i < lhs->GetChildren().size(); i++) {
        if (lhs->GetChildren()[i] != rhs->GetChildren()[i])
            return false;
    }
    return true;
}

// Initialize static variables
MetaNodePtr MetaNode::terminalZero;
MetaNodePtr MetaNode::terminalOne;
bool MetaNode::zeroInit = false;
bool MetaNode::oneInit = false;

} // end of aomdd namespace
