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
AOMDDFunction::AOMDDFunction() : fullReduce(true) {
    root = MetaNode::GetOne();
}
AOMDDFunction::AOMDDFunction(const Scope &domainIn) : Function(domainIn) {
}

AOMDDFunction::AOMDDFunction(const Scope &domainIn, const std::vector<double> &valsIn)
    : Function(domainIn) {
    root = mgr->CreateMetaNode(domain, valsIn);
}

AOMDDFunction::AOMDDFunction(const Scope &domainIn,
        const PseudoTree *pseudoTree, const std::vector<double> &valsIn, bool fr) :
    Function(domainIn), pt(pseudoTree), fullReduce(fr) {
    if (pt->HasDummy()) {
        Scope s(domain);
        s.AddVar(pt->GetRoot(), 1);
        list<int> ordering = s.GetOrdering();
        ordering.pop_back();
        ordering.push_front(pt->GetRoot());
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
    /*
    if (pt->HasDummy()) {
        s.AddVar(pt->GetRoot(), 1);
        list<int> ordering = s.GetOrdering();
        ordering.pop_back();
        ordering.push_front(pt->GetRoot());
        s.SetOrdering(ordering);
    }
    */
    DirectedGraph embedpt;
    int embedroot;
    tie(embedpt, embedroot) = pt->GenerateEmbeddable(s);

    vector<MetaNodePtr> lhsParam(1, root);
    vector<MetaNodePtr> rhsParam(1, rhs.root);
    vector<ApplyParamSet> apsVec = mgr->GetParamSets(embedpt, lhsParam, rhsParam);
    ApplyParamSet aps;
    if (apsVec.size() == 1) {
        aps = apsVec[0];
    }
    else {
        DInEdge ei, ei_end;
        tie(ei, ei_end) = in_edges(root->GetVarID(), embedpt);
        if (ei != ei_end) {
            cout << "Generating dummy for apply (in AOMDD Multiply)" << endl;
            int dummyID = source(*ei, embedpt);
            vector<MetaNodePtr> mCh;
            mCh.push_back(root);
            ANDNodePtr dummyAND(new MetaNode::ANDNode(1.0, mCh));
            vector<ANDNodePtr> aCh(1, dummyAND);
            MetaNodePtr dummy = mgr->CreateMetaNode(dummyID, 1, aCh);
            lhsParam[0] = dummy;
            apsVec = mgr->GetParamSets(embedpt, lhsParam, rhsParam);
            assert(apsVec.size() == 1);
            aps = apsVec[0];
        }
    }
    cout << "lhs: " << aps.first->GetVarID() << ", rhs:";
    BOOST_FOREACH(MetaNodePtr i, aps.second) {
        cout << " " << i->GetVarID();
    }
    cout << endl;
    if (fullReduce) {
        double w = 1.0;
        root = mgr->FullReduce(mgr->Apply(aps.first, aps.second, PROD, embedpt), w)[0];
    }
    else {
        root = mgr->Apply(aps.first, aps.second, PROD, embedpt);
    }
    domain = s;
}

void AOMDDFunction::Marginalize(const Scope &elimVars) {
    if (fullReduce) {
        double w = 1.0;
        root = mgr->FullReduce(mgr->Marginalize(root, elimVars, pt->GetTree()), w)[0];
    }
    else {
        root = mgr->Marginalize(root, elimVars, pt->GetTree());
    }
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

void AOMDDFunction::PrintAsTable(ostream &out) const {
    Assignment a(domain);
    a.SetAllVal(0);
    do {
        a.Save(out); out << " value=" << GetVal(a) << endl;
    } while (a.Iterate());
}

NodeManager *AOMDDFunction::mgr = NodeManager::GetNodeManager();

}
