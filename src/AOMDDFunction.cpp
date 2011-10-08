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
    root = WeightedMetaNodeList(MetaNodeList(1, MetaNode::GetOne()), 1.0);
}
AOMDDFunction::AOMDDFunction(const Scope &domainIn) : Function(domainIn) {
}

AOMDDFunction::AOMDDFunction(const Scope &domainIn, const std::vector<double> &valsIn)
    : Function(domainIn) {
    root = mgr->CreateMetaNode(domain, valsIn);
}

AOMDDFunction::AOMDDFunction(const Scope &domainIn,
        const PseudoTree *pseudoTree, const std::vector<double> &valsIn) :
    Function(domainIn), pt(pseudoTree) {
    /*
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
    */
    root = mgr->CreateMetaNode(domain, valsIn);
}

AOMDDFunction::AOMDDFunction(const AOMDDFunction &f)
	: Function(f), root(f.root), pt(f.pt) {
}

double AOMDDFunction::GetVal(const Assignment &a, bool logOut) const {
    double value = root.second;
    BOOST_FOREACH(MetaNodePtr m, root.first) {
        value *= m->Evaluate(a);
    }
    return logOut ? log10(value) : value;
}

bool AOMDDFunction::SetVal(const Assignment &a, double val) {
    // Maybe later
    return false;
}

void AOMDDFunction::Multiply(const AOMDDFunction& rhs) {
//    Save(cout); cout << endl;
//    rhs.Save(cout); cout << endl;
    Scope s = domain + rhs.GetScope();
    if (root.first.size() == 1) {
        if (root.first[0].get() == MetaNode::GetZero().get()) {
            domain = s;
            return;
        }
        else if (root.first[0].get() == MetaNode::GetOne().get()) {
            double w = root.second;
            root = rhs.root;
            root.second *= w;

            domain = s;
            return;
        }
    }
    if (rhs.root.first.size() == 1) {
        if (rhs.root.first[0].get() == MetaNode::GetZero().get()) {
            root = rhs.root;
            domain = s;
            return;
        }
        else if (rhs.root.first[0].get() == MetaNode::GetOne().get()) {
            root.second *= rhs.root.second;
            domain = s;
            return;
        }
    }

    /*
    if (s.IsEmpty()) {
        s.AddVar(root->GetVarID(), 1);
        s.AddVar(rhs.root->GetVarID(), 1);
    }
    */
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

    /*
    s.Save(cout); cout << endl;

    string debugDot = "debug.dot";
    WriteDot(embedpt, debugDot);
    */

    /*
    vector<MetaNodePtr> lhsParam(1, root);
    vector<MetaNodePtr> rhsParam(1, rhs.root);
    */
//    cout << "original lhs: " << root->GetVarID()
//            << ", original rhs: " << rhs.root->GetVarID() << endl;
    vector<ApplyParamSet> apsVec = mgr->GetParamSets(embedpt, root.first, rhs.root.first);
    /*
    ApplyParamSet aps;
    if (apsVec.size() == 1) {
        aps = apsVec[0];
    }
    else {
        DInEdge ei, ei_end;
        tie(ei, ei_end) = in_edges(root->GetVarID(), embedpt);
        if (ei != ei_end) {
//            cout << "Generating dummy for apply (in AOMDD Multiply)" << endl;
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
    */

    /*
    cout << "lhs: " << aps.first->GetVarID() << ", ";
    cout << "rhs:";
    BOOST_FOREACH(MetaNodePtr r, aps.second) {
        cout << " " << r->GetVarID();
    }
    cout << endl;
    aps.first->RecursivePrint(cout); cout << endl;
    */
    root.first.clear();
    root.second *= rhs.root.second;
    bool terminalOnly = true;
    bool earlyTerminate = false;

    BOOST_FOREACH(ApplyParamSet aps, apsVec) {
        WeightedMetaNodeList subDD = mgr->Apply(aps.first, aps.second, PROD, embedpt);

        root.second *= subDD.second;
        BOOST_FOREACH(MetaNodePtr m, subDD.first) {
            if (m.get() == MetaNode::GetZero().get()) {
                root.first.clear();
                root.first.push_back(m);
                earlyTerminate = true;
                break;
            }
            if (!m->IsTerminal()) {
                if (terminalOnly) {
                    root.first.clear();
                    terminalOnly = false;
                }
                root.first.push_back(m);
            }
            else if (root.first.empty()) {
                root.first.push_back(m);
            }
        }
        if (earlyTerminate) break;
    }
    domain = s;
    NodeManager::GetNodeManager()->UTGarbageCollect();
}

void AOMDDFunction::Marginalize(const Scope &elimVars, bool mutableIDs) {
    /*
    domain.Save(cout); cout << endl;
    elimVars.Save(cout); cout << endl;
    */
    Scope actualElimVars = domain * elimVars;
    if (root.first.size() == 1 && root.first[0]->IsTerminal()) {
//        cout << "Using special terminal case" << endl;
        domain = domain - actualElimVars;
        root.second *= actualElimVars.GetCard();
        return;
    }

    int elimvar = actualElimVars.GetOrdering().front();
    /*
    if (fullReduce) {
        root = mgr->FullReduce(mgr->Marginalize(root, elimVars, pt->GetTree()));
        domain = domain - elimVars;
        if (mutableIDs && root->IsDummy() && !domain.IsEmpty() && !domain.VarExists(root->GetVarID())) {
            int newDummyID = domain.GetOrdering().front();
            cout << "Changing dummy id " << "<" << root->GetVarID() << "> to <" << newDummyID << ">" << endl;
            root = mgr->CreateMetaNode(newDummyID, 1.0, root->GetChildren());
        }
    }
    */
    WeightedMetaNodeList newroot;
    newroot.second = root.second;
    bool sumOpPerformed = false;
    // Build an "elim chain"
    // From the elim variable, trace the parents until it hits one of the metanodes of the root
    // If it isn't reached, no need to marginalize
    // otherwise, pass the chain in.
    // It will also guide the marginalization to directly go to the nodes to be eliminated

    DirectedGraph elimChain;
    DInEdge ei, ei_end;
    tie(ei, ei_end) = in_edges(elimvar, pt->GetTree());
    int chainRoot = elimvar;

    // Hackish way to add a vertex to the graph
    add_edge(chainRoot, chainRoot+1, elimChain);
    remove_edge(chainRoot, chainRoot+1, elimChain);

    bool chainDone = false;
    BOOST_FOREACH(MetaNodePtr m, root.first) {
        if (chainRoot == m->GetVarID()) {
            chainDone = true;
        }
    }
    while (!chainDone && ei != ei_end) {
        int current = target(*ei, pt->GetTree());
        chainRoot = source(*ei, pt->GetTree());
        add_edge(chainRoot, current, elimChain);
        BOOST_FOREACH(MetaNodePtr m, root.first) {
	        if (chainRoot == m->GetVarID()) {
	            chainDone = true;
	        }
        }
        tie(ei, ei_end) = in_edges(chainRoot, pt->GetTree());
    }

    cout << endl;
    BOOST_FOREACH(MetaNodePtr m, root.first) {
        if (m->GetVarID() == chainRoot) {
            WeightedMetaNodeList l = mgr->Marginalize(m, elimVars, elimChain, sumOpPerformed);
            newroot.first.insert(newroot.first.end(), l.first.begin(), l.first.end());
            newroot.second *= l.second;
        }
        else {
            newroot.first.push_back(m);
        }
    }
    /*
    if (!sumOpPerformed) {
        newroot.second *= actualElimVars.GetCard();
    }
    */
    root = newroot;
    domain = domain - actualElimVars;
    NodeManager::GetNodeManager()->UTGarbageCollect();
}

void AOMDDFunction::Maximize(const Scope &elimVars, bool mutableIDs) {
    /*
    domain.Save(cout); cout << endl;
    elimVars.Save(cout); cout << endl;
    */
    if (root.first.size() == 1 && root.first[0]->IsTerminal()) {
        domain = domain - elimVars;
        return;
    }
    /*
    if (fullReduce) {
        root = mgr->FullReduce(mgr->Maximize(root, elimVars, pt->GetTree()));
//        root = mgr->Maximize(root, elimVars, pt->GetTree());
        domain = domain - elimVars;
        if (mutableIDs && root->IsDummy() && !domain.IsEmpty() && !domain.VarExists(root->GetVarID())) {
            int newDummyID = domain.GetOrdering().front();
            cout << "Changing dummy id " << "<" << root->GetVarID() << "> to <" << newDummyID << ">" << endl;
            root = mgr->CreateMetaNode(newDummyID, 1.0, root->GetChildren());
        }
    }
    */
    WeightedMetaNodeList newroot;
    newroot.second = root.second;
    BOOST_FOREACH(MetaNodePtr m, root.first) {
        WeightedMetaNodeList l = mgr->Maximize(m, elimVars, pt->GetTree());
        newroot.first.insert(newroot.first.end(), l.first.begin(), l.first.end());
        newroot.second *= l.second;
    }
    root = newroot;
    domain = domain - elimVars;
    NodeManager::GetNodeManager()->UTGarbageCollect();
}

double AOMDDFunction::Maximum(const Assignment &cond) {
    double res = root.second;
    BOOST_FOREACH(MetaNodePtr m, root.first) {
        res *= m->Maximum(cond);
    }
    return res;
}

double AOMDDFunction::Sum(const Assignment &cond) {
    double res = root.second;
    BOOST_FOREACH(MetaNodePtr m, root.first) {
        res *= m->Sum(cond);
    }
    return res;
}

void AOMDDFunction::Condition(const Assignment &cond) {
    if (root.first.size() == 1 && root.first[0]->IsTerminal()) {
        domain = domain - cond;
        return;
    }
    /*
    if (fullReduce) {
        double w = 1.0;
        root = mgr->FullReduce(mgr->Condition(root, cond), w)[0];
        domain = domain - cond;
        if (root->IsTerminal()) {
//            cout << "varid (became terminal): " << varid << endl;
            int dummy;
            // Terminal is still only within the scope of its function
            if (!domain.IsEmpty()) {
                dummy = domain.GetOrdering().back();
            }
            // Terminal affects global result if scope became empty
            else {
                dummy = pt->GetRoot();
            }
            ANDNodePtr singleAND(new MetaNode::ANDNode(w, vector<MetaNodePtr>(1, root)));
            root = mgr->CreateMetaNode(dummy, 1, vector<ANDNodePtr>(1, singleAND));
            Scope s;
            s.AddVar(dummy, pt->GetScope().GetVarCard(dummy));
//            domain = s;
        }
    }
    */
    WeightedMetaNodeList newroot;
    newroot.second = root.second;
    BOOST_FOREACH(MetaNodePtr m, root.first) {
        WeightedMetaNodeList l = mgr->Condition(m, cond);
        newroot.first.insert(newroot.first.end(), l.first.begin(), l.first.end());
        newroot.second *= l.second;
    }
    root = newroot;
    domain = domain - cond;
}

AOMDDFunction::~AOMDDFunction() {
}

void AOMDDFunction::Save(ostream &out) const {
    cout << "Global Weight: " << root.second << endl;
    BOOST_FOREACH(MetaNodePtr m, root.first) {
        m->RecursivePrint(out);
    }
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
