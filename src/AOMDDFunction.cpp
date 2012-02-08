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
//    root = make_shared<ANDNode>(1.0, vector<MetaNodePtr>(1, MetaNode::GetOne()));
    root = ANDNodePtr(new MetaNode::ANDNode(1.0, vector<MetaNodePtr>(1, MetaNode::GetOne())));
//    root = WeightedMetaNodeList(MetaNodeList(1, MetaNode::GetOne()), 1.0);
}
AOMDDFunction::AOMDDFunction(const Scope &domainIn) : Function(domainIn) {
}

AOMDDFunction::AOMDDFunction(const Scope &domainIn, const std::vector<double> &valsIn)
    : Function(domainIn) {
    root = mgr->CreateMetaNode(domain, valsIn);
    BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
        m->AddParent(root.get());
    }
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
    BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
        m->AddParent(root.get());
    }
}

AOMDDFunction::AOMDDFunction(const AOMDDFunction &f)
	: Function(f), root(f.root), pt(f.pt) {
    BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
        m->AddParent(root.get());
    }
}

double AOMDDFunction::GetVal(const Assignment &a, bool logOut) const {
    double value = root->Evaluate(a);
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
    if (root->GetChildren().size() == 1) {
        if (root->GetChildren()[0].get() == MetaNode::GetZero().get()) {
            domain = s;
            return;
        }
        else if (root->GetChildren()[0].get() == MetaNode::GetOne().get()) {
            double w = root->GetWeight();
            root = rhs.root;
            root->SetWeight(root->GetWeight()*w);

            domain = s;
            return;
        }
    }
    if (rhs.root->GetChildren().size() == 1) {
        if (rhs.root->GetChildren()[0].get() == MetaNode::GetZero().get()) {
            root = rhs.root;
            domain = s;
            return;
        }
        else if (rhs.root->GetChildren()[0].get() == MetaNode::GetOne().get()) {
            root->SetWeight(root->GetWeight()*rhs.root->GetWeight());
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
    vector<ApplyParamSet> apsVec = mgr->GetParamSets(embedpt, root->GetChildren(), rhs.root->GetChildren());
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

    ANDNodePtr newroot(new ANDNode());
//    ANDNodePtr newroot(new ANDNode());
    newroot->SetWeight(root->GetWeight()*rhs.root->GetWeight());
//    root->GetChildren().clear();
//    root->SetWeight(root->GetWeight()*rhs.root->GetWeight());
    bool terminalOnly = true;
    bool earlyTerminate = false;

    BOOST_FOREACH(ApplyParamSet aps, apsVec) {
        ANDNodePtr subDD = mgr->Apply(aps.first, aps.second, PROD, embedpt);

        newroot->ScaleWeight(subDD->GetWeight());
        BOOST_FOREACH(MetaNodePtr m, subDD->GetChildren()) {
            if (m.get() == MetaNode::GetZero().get()) {
                newroot->GetChildren().clear();
                newroot->GetChildren().push_back(m);
                earlyTerminate = true;
                break;
            }
            if (!m->IsTerminal()) {
                if (terminalOnly) {
                    newroot->GetChildren().clear();
                    terminalOnly = false;
                }
                newroot->GetChildren().push_back(m);
                m->AddParent(newroot.get());
            }
            else if (newroot->GetChildren().empty()) {
                newroot->GetChildren().push_back(m);
                m->AddParent(newroot.get());
            }
        }
        if (earlyTerminate) break;
    }
    domain = s;
    root = newroot;
    NodeManager::GetNodeManager()->UTGarbageCollect();
}

void AOMDDFunction::Marginalize(const Scope &elimVars, bool mutableIDs) {
    /*
    domain.Save(cout); cout << endl;
    elimVars.Save(cout); cout << endl;
    */
    Scope actualElimVars = domain * elimVars;
    if (root->GetChildren().size() == 1 && root->GetChildren()[0]->IsTerminal()) {
//        cout << "Using special terminal case" << endl;
        domain = domain - actualElimVars;
        root->ScaleWeight(actualElimVars.GetCard());
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
    /*
    WeightedMetaNodeList newroot;
    newroot.second = root.second;
    */
    ANDNodePtr newroot(new ANDNode());
    newroot->SetWeight(root->GetWeight());
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
    BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
        if (chainRoot == m->GetVarID()) {
            chainDone = true;
        }
    }
    while (!chainDone && ei != ei_end) {
        int current = target(*ei, pt->GetTree());
        chainRoot = source(*ei, pt->GetTree());

        // keep traversing up if variable is not in the scope of this function
        while (!domain.VarExists(chainRoot)) {
            DInEdge e, e_end;
            tie(e, e_end) = in_edges(chainRoot, pt->GetTree());
            chainRoot = source(*e, pt->GetTree());
        }
        add_edge(chainRoot, current, elimChain);
        BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
	        if (chainRoot == m->GetVarID()) {
	            chainDone = true;
	        }
        }
        tie(ei, ei_end) = in_edges(chainRoot, pt->GetTree());
    }

    BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
        if (m->GetVarID() == chainRoot) {
            ANDNodePtr l = mgr->Marginalize(m, elimVars, elimChain);
            newroot->GetChildren().insert(newroot->GetChildren().end(), l->GetChildren().begin(), l->GetChildren().end());
            newroot->SetWeight(newroot->GetWeight()*l->GetWeight());
        }
        else {
            newroot->GetChildren().push_back(m);
        }
    }
    root = newroot;
    domain = domain - actualElimVars;
    NodeManager::GetNodeManager()->UTGarbageCollect();
}

void AOMDDFunction::Maximize(const Scope &elimVars, bool mutableIDs) {
    /*
    domain.Save(cout); cout << endl;
    elimVars.Save(cout); cout << endl;
    */
    if (root->GetChildren().size() == 1 && root->GetChildren()[0]->IsTerminal()) {
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
    /*
    WeightedMetaNodeList newroot;
    newroot.second = root.second;
    */
    ANDNodePtr newroot;
    newroot->SetWeight(root->GetWeight());
    BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
        ANDNodePtr l = mgr->Maximize(m, elimVars, pt->GetTree());
        newroot->GetChildren().insert(newroot->GetChildren().end(), l->GetChildren().begin(), l->GetChildren().end());
        newroot->SetWeight(newroot->GetWeight()*l->GetWeight());
    }
    root = newroot;
    domain = domain - elimVars;
    NodeManager::GetNodeManager()->UTGarbageCollect();
}

void AOMDDFunction::MarginalizeFast(const Scope &elimVars, bool mutableIDs) {
    Scope actualElimVars = domain * elimVars;
    if (root->GetChildren().size() == 1 && root->GetChildren()[0]->IsTerminal()) {
//        cout << "Using special terminal case" << endl;
        domain = domain - actualElimVars;
        root->ScaleWeight(actualElimVars.GetCard());
        return;
    }

    int elimvar = actualElimVars.GetOrdering().front();
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
    DirectedGraph elimChain;
    DInEdge ei, ei_end;
    tie(ei, ei_end) = in_edges(elimvar, pt->GetTree());
    int chainRoot = elimvar;

    // Hackish way to add a vertex to the graph
    add_edge(chainRoot, chainRoot+1, elimChain);
    remove_edge(chainRoot, chainRoot+1, elimChain);

    bool chainDone = false;
    BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
        if (chainRoot == m->GetVarID()) {
            chainDone = true;
        }
    }
    while (!chainDone && ei != ei_end) {
        int current = target(*ei, pt->GetTree());
        chainRoot = source(*ei, pt->GetTree());

        // keep traversing up if variable is not in the scope of this function
        while (!domain.VarExists(chainRoot)) {
            DInEdge e, e_end;
            tie(e, e_end) = in_edges(chainRoot, pt->GetTree());
            chainRoot = source(*e, pt->GetTree());
        }
        add_edge(chainRoot, current, elimChain);
        BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
	        if (chainRoot == m->GetVarID()) {
	            chainDone = true;
	        }
        }
        tie(ei, ei_end) = in_edges(chainRoot, pt->GetTree());
    }


    set<int> relevantVars;
    relevantVars.insert(chainRoot);
    DEdge eoi, eoi_end;
    tie(eoi, eoi_end) = out_edges(chainRoot, elimChain);
    while (eoi != eoi_end) {
        int child = target(*eoi, elimChain);
        relevantVars.insert(child);
        tie(eoi, eoi_end) = out_edges(child, elimChain);
    }
    bool foundRelevant = false;
    for (size_t i = 0; i < root->GetChildren().size(); ++i) {
        MetaNodePtr &m = root->GetChildren()[i];
        if (relevantVars.find(m->GetVarID()) == relevantVars.end()) {
            continue;
        }
        foundRelevant = true;
        mgr->MarginalizeFast(m, elimVars, relevantVars);
        /*
        if (elimVars.VarExists(m->GetVarID())) {
            double w = 0.0;
            vector<ANDNodePtr> &andNodesJ = m->GetChildren();
            for (unsigned int k = 0; k < andNodesJ.size(); ++k) {
                w += andNodesJ[k]->GetWeight();
            }
            if (w == 0) {
                root.first.clear();
                root.first.push_back(MetaNode::GetZero());
            }
            else {
                m = MetaNode::GetOne();
            }
            root.second *= w;
        }
        else {
            // First check if the ANDNodes connect to relevant variables
            BOOST_FOREACH(ANDNodePtr a, m->GetChildren()) {
                bool hasRelevantChild = false;
                BOOST_FOREACH(MetaNodePtr mm, a->GetChildren()) {
                    if (relevantVars.find(mm->GetVarID()) !=
                            relevantVars.end()) {
                        hasRelevantChild = true;
                        break;
                    }
                }
                // multiply by elimination var scope size if not
                // and normalize
                if (!hasRelevantChild) {
	                a->SetWeight(a->GetWeight() * actualElimVars.GetVarCard(elimvar));
                }
            }
            mgr->MarginalizeFast(m, elimVars, relevantVars);
        }
        */
    }

    if (!foundRelevant) root->ScaleWeight(actualElimVars.GetCard());

    // Clean up root of terminals if root.first.size() > 1
    while (root->GetChildren().size() > 1) {
        for (size_t j = 0; j < root->GetChildren().size(); ++j) {
            if (root->GetChildren()[j]->IsTerminal()) {
                root->GetChildren().erase(root->GetChildren().begin() + j);
                break;
            }
        }
    }
    domain = domain - elimVars;
    NodeManager::GetNodeManager()->UTGarbageCollect();
}

void AOMDDFunction::MaximizeFast(const Scope &elimVars, bool mutableIDs) {
    Scope actualElimVars = domain * elimVars;
    if (root->GetChildren().size() == 1 && root->GetChildren()[0]->IsTerminal()) {
//        cout << "Using special terminal case" << endl;
        domain = domain - actualElimVars;
        return;
    }

    int elimvar = actualElimVars.GetOrdering().front();
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
    DirectedGraph elimChain;
    DInEdge ei, ei_end;
    tie(ei, ei_end) = in_edges(elimvar, pt->GetTree());
    int chainRoot = elimvar;

    // Hackish way to add a vertex to the graph
    add_edge(chainRoot, chainRoot+1, elimChain);
    remove_edge(chainRoot, chainRoot+1, elimChain);

    bool chainDone = false;
    BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
        if (chainRoot == m->GetVarID()) {
            chainDone = true;
        }
    }
    while (!chainDone && ei != ei_end) {
        int current = target(*ei, pt->GetTree());
        chainRoot = source(*ei, pt->GetTree());

        // keep traversing up if variable is not in the scope of this function
        while (!domain.VarExists(chainRoot)) {
            DInEdge e, e_end;
            tie(e, e_end) = in_edges(chainRoot, pt->GetTree());
            chainRoot = source(*e, pt->GetTree());
        }
        add_edge(chainRoot, current, elimChain);
        BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
	        if (chainRoot == m->GetVarID()) {
	            chainDone = true;
	        }
        }
        tie(ei, ei_end) = in_edges(chainRoot, pt->GetTree());
    }

    // Enumerate the set of relevant variables
    set<int> relevantVars;
    relevantVars.insert(chainRoot);
    DEdge eoi, eoi_end;
    tie(eoi, eoi_end) = out_edges(chainRoot, elimChain);
    while (eoi != eoi_end) {
        int child = target(*eoi, elimChain);
        relevantVars.insert(child);
        tie(eoi, eoi_end) = out_edges(child, elimChain);
    }

    for (size_t i = 0; i < root->GetChildren().size(); ++i) {
        MetaNodePtr &m = root->GetChildren()[i];

        // Skip irrelevant variables
        if (relevantVars.find(m->GetVarID()) == relevantVars.end()) {
            continue;
        }
        mgr->MaximizeFast(m, elimVars, relevantVars);
    }

    // Clean up root of terminals if root.first.size() > 1
    while (root->GetChildren().size() > 1) {
        for (size_t j = 0; j < root->GetChildren().size(); ++j) {
            if (root->GetChildren()[j]->IsTerminal()) {
                root->GetChildren().erase(root->GetChildren().begin() + j);
                break;
            }
        }
    }
    domain = domain - elimVars;
    NodeManager::GetNodeManager()->UTGarbageCollect();
}

void AOMDDFunction::Minimize(const Scope &elimVars, bool mutableIDs) {
    /*
    domain.Save(cout); cout << endl;
    elimVars.Save(cout); cout << endl;
    */
    if (root->GetChildren().size() == 1 && root->GetChildren()[0]->IsTerminal()) {
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
    /*
    WeightedMetaNodeList newroot;
    newroot.second = root.second;
    */
//    ANDNodePtr newroot = make_shared<ANDNode>();
    ANDNodePtr newroot(new ANDNode());
    newroot->SetWeight(root->GetWeight());

    BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
        ANDNodePtr l = mgr->Minimize(m, elimVars, pt->GetTree());
        newroot->GetChildren().insert(newroot->GetChildren().end(), l->GetChildren().begin(), l->GetChildren().end());
        newroot->SetWeight(l->GetWeight());
    }
    root = newroot;
    domain = domain - elimVars;
    NodeManager::GetNodeManager()->UTGarbageCollect();
}

double AOMDDFunction::Maximum(const Assignment &cond) {
    double res = root->GetWeight();
    BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
        res *= m->Maximum(cond);
    }
    return res;
}

double AOMDDFunction::Sum(const Assignment &cond) {
    double res = root->GetWeight();
    BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
        res *= m->Sum(cond);
    }
    return res;
}

void AOMDDFunction::ConditionFast(const Assignment &cond) {
    Scope actualElimVars = domain * cond;
    if (root->GetChildren().size() == 1 && root->GetChildren()[0]->IsTerminal()) {
        domain = domain - cond;
        return;
    }

    int elimvar = actualElimVars.GetOrdering().front();

    DirectedGraph elimChain;
    DInEdge ei, ei_end;
    tie(ei, ei_end) = in_edges(elimvar, pt->GetTree());
    int chainRoot = elimvar;

    // Hackish way to add a vertex to the graph
    add_edge(chainRoot, chainRoot+1, elimChain);
    remove_edge(chainRoot, chainRoot+1, elimChain);

    bool chainDone = false;
    BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
        if (chainRoot == m->GetVarID()) {
            chainDone = true;
        }
    }
    while (!chainDone && ei != ei_end) {
        int current = target(*ei, pt->GetTree());
        chainRoot = source(*ei, pt->GetTree());

        // keep traversing up if variable is not in the scope of this function
        while (!domain.VarExists(chainRoot)) {
            DInEdge e, e_end;
            tie(e, e_end) = in_edges(chainRoot, pt->GetTree());
            chainRoot = source(*e, pt->GetTree());
        }
        add_edge(chainRoot, current, elimChain);
        BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
	        if (chainRoot == m->GetVarID()) {
	            chainDone = true;
	        }
        }
        tie(ei, ei_end) = in_edges(chainRoot, pt->GetTree());
    }

    // Enumerate the set of relevant variables
    set<int> relevantVars;
    relevantVars.insert(chainRoot);
    DEdge eoi, eoi_end;
    tie(eoi, eoi_end) = out_edges(chainRoot, elimChain);
    while (eoi != eoi_end) {
        int child = target(*eoi, elimChain);
        relevantVars.insert(child);
        tie(eoi, eoi_end) = out_edges(child, elimChain);
    }

    for (size_t i = 0; i < root->GetChildren().size(); ++i) {
        MetaNodePtr &m = root->GetChildren()[i];

        // Skip irrelevant variables
        if (relevantVars.find(m->GetVarID()) == relevantVars.end()) {
            continue;
        }
        mgr->ConditionFast(m, cond, relevantVars);
    }

    // Clean up root of terminals if root.first.size() > 1
    while (root->GetChildren().size() > 1) {
        for (size_t j = 0; j < root->GetChildren().size(); ++j) {
            if (root->GetChildren()[j]->IsTerminal()) {
                root->GetChildren().erase(root->GetChildren().begin() + j);
                break;
            }
        }
    }
    domain = domain - cond;
    NodeManager::GetNodeManager()->UTGarbageCollect();

}

void AOMDDFunction::Condition(const Assignment &cond) {
    if (root->GetChildren().size() == 1 && root->GetChildren()[0]->IsTerminal()) {
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
    /*
    WeightedMetaNodeList newroot;
    newroot.second = root.second;
    */
//    ANDNodePtr newroot = make_shared<ANDNode>();
    ANDNodePtr newroot(new ANDNode());
    newroot->SetWeight(root->GetWeight());
    BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
        ANDNodePtr l = mgr->Condition(m, cond);
        newroot->GetChildren().insert(newroot->GetChildren().end(), l->GetChildren().begin(), l->GetChildren().end());
//        newroot.second *= l.second;
        newroot->SetWeight(newroot->GetWeight()*l->GetWeight());
    }
    root = newroot;
    domain = domain - cond;
}

AOMDDFunction::~AOMDDFunction() {
}

void AOMDDFunction::Save(ostream &out) const {
    cout << "Global Weight: " << root->GetWeight() << endl;
    BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
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
