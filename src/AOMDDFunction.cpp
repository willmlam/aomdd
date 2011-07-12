/*
 *  AOMDDFunction.cpp
 *  aomdd
 *
 *  Created by William Lam on Jun 23, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#include "AOMDDFunction.h"
using namespace std;

namespace aomdd {
AOMDDFunction::AOMDDFunction() {
    // TODO Auto-generated constructor stub

}
AOMDDFunction::AOMDDFunction(const Scope &domainIn) : Function(domainIn) {
}

AOMDDFunction::AOMDDFunction(const Scope &domainIn, const std::vector<double> &valsIn)
    : Function(domainIn) {
    root = mgr->CreateMetaNode(domain, valsIn);
}

AOMDDFunction::AOMDDFunction(const Scope &domainIn,
        const PseudoTree &pseudoTree, const std::vector<double> &valsIn) :
    Function(domainIn), pt(pseudoTree) {
    if (pt.HasDummy()) {
        Scope s(domain);
        s.AddVar(pt.GetRoot(), 1);
        list<int> ordering = s.GetOrdering();
        ordering.pop_back();
        ordering.push_front(pt.GetRoot());
        s.SetOrdering(ordering);
        root = mgr->CreateMetaNode(s, valsIn);
    }
    else {
        root = mgr->CreateMetaNode(domain, valsIn);
    }
}

double AOMDDFunction::GetVal(const Assignment &a, bool logOut) const {
    double value = root->Evaluate(a);
    return logOut ? log(value) : value;
}

bool AOMDDFunction::SetVal(const Assignment &a, double val) {
    // Maybe later
    return false;
}

void AOMDDFunction::Multiply(const AOMDDFunction& rhs) {
    Scope s = domain + rhs.GetScope();
    if (pt.HasDummy()) {
        s.AddVar(pt.GetRoot(), 1);
        list<int> ordering = s.GetOrdering();
        ordering.pop_back();
        ordering.push_front(pt.GetRoot());
        s.SetOrdering(ordering);
    }
    DirectedGraph embedpt;
    int embedroot;
    tie(embedpt, embedroot) = pt.GenerateEmbeddable(s);

    vector<MetaNodePtr> rhsParam(1, rhs.root);
    double w = 1.0;
    root = mgr->FullReduce(mgr->Apply(root, rhsParam, PROD, embedpt), w)[0];
    domain = s;
}

void AOMDDFunction::Marginalize(const Scope &elimVars) {
    double w = 1.0;
    root = mgr->FullReduce(mgr->Marginalize(root, elimVars, pt.GetTree()), w)[0];
    domain = domain - elimVars;
}

void AOMDDFunction::Normalize() {
    root = mgr->Normalize(root);
}

AOMDDFunction::~AOMDDFunction() {
    // TODO Auto-generated destructor stub
}

void AOMDDFunction::Save(ostream &out) const {
    root->RecursivePrint(out);
}

NodeManager *AOMDDFunction::mgr = NodeManager::GetNodeManager();

}
