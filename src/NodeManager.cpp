/*
 *  NodeManager.cpp
 *  aomdd
 *
 *  Created by William Lam on May 13, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#include "MetaNode.h"
#include "NodeManager.h"
#include "utils.h"

namespace aomdd {
using namespace std;

// Be sure to test to see if sets give the same hash if items are inserted
// in different orders
size_t hash_value(const Operation &o) {
    size_t seed = 0;
    boost::hash_combine(seed, o.GetOperator());
    BOOST_FOREACH(ParamSet::value_type i, o.GetParamSet()) {
        boost::hash_combine(seed, i.get());
    }
    return seed;

}
bool operator==(const Operation &lhs, const Operation &rhs) {
    return lhs.GetOperator() == rhs.GetOperator() && lhs.GetParamSet()
            == rhs.GetParamSet();
}

// =================

NodeManager *NodeManager::GetNodeManager() {
    if (!initialized) {
        singleton = new NodeManager();
        initialized = true;
        return singleton;
    }
    else {
        return singleton;
    }
}

vector<MetaNodePtr> NodeManager::ReweighNodes(const vector<MetaNodePtr> &nodes, double w) {
    vector<MetaNodePtr> ret;
    BOOST_FOREACH(MetaNodePtr i, nodes) {
        if (i.get() != MetaNode::GetZero().get() &&
                i.get() != MetaNode::GetOne().get()) {
            MetaNodePtr newNode(new MetaNode(*i));
            newNode->SetWeight(newNode->GetWeight() * w);
            ret.push_back(newNode);
        }
        else {
            ret.push_back(i);
        }
    }
    return ret;
}

vector<MetaNodePtr> NodeManager::CopyMetaNodes(const vector<MetaNodePtr> &nodes) {
    vector<MetaNodePtr> ret;
    BOOST_FOREACH(MetaNodePtr i, nodes) {
        if (i.get() != MetaNode::GetZero().get() &&
                i.get() != MetaNode::GetOne().get()) {
            MetaNodePtr newNode(new MetaNode(*i));
            ret.push_back(newNode);
        }
        else {
            ret.push_back(i);
        }
    }
    return ret;
}

vector<ANDNodePtr> NodeManager::CopyANDNodes(const vector<ANDNodePtr> &nodes) {
    vector<ANDNodePtr> ret;
    BOOST_FOREACH(ANDNodePtr i, nodes) {
        ANDNodePtr newNode(new MetaNode::ANDNode(*i));
        ret.push_back(newNode);
    }
    return ret;
}

vector<ApplyParamSet> NodeManager::GetParamSets(const DirectedGraph &tree,
        const vector<MetaNodePtr> &lhs, const vector<MetaNodePtr> &rhs) const {
    vector<ApplyParamSet> ret;

    unordered_map<int, MetaNodePtr> lhsMap;
    unordered_map<int, MetaNodePtr> rhsMap;

    BOOST_FOREACH(MetaNodePtr i, lhs) {
        lhsMap[i->GetVarID()] = i;
    }
    BOOST_FOREACH(MetaNodePtr i, rhs) {
        rhsMap[i->GetVarID()] = i;
    }

    unordered_map<int, int> hiAncestor;
    BOOST_FOREACH(MetaNodePtr i, lhs) {
        int varid = i->GetVarID();
        hiAncestor[varid] = varid;
        if (varid == -1) continue;

        int parent = varid;
        DInEdge ei, ei_end;
        tie(ei, ei_end) = in_edges(parent, tree);
        while (ei != ei_end) {
            parent = source(*ei, tree);
            if (rhsMap.find(parent) != rhsMap.end()) {
                hiAncestor[varid] = parent;
            }
            tie(ei, ei_end) = in_edges(parent, tree);
        }
    }

    BOOST_FOREACH(MetaNodePtr i, rhs) {
        int varid = i->GetVarID();
        hiAncestor[varid] = varid;
        if (varid == -1) continue;

        int parent = varid;
        DInEdge ei, ei_end;
        tie(ei, ei_end) = in_edges(parent, tree);
        while (ei != ei_end) {
            parent = source(*ei, tree);
            if (lhsMap.find(parent) != lhsMap.end()) {
                hiAncestor[varid] = parent;
            }
            tie(ei, ei_end) = in_edges(parent, tree);
        }
    }

    unordered_map<int, vector<int> > descendants;
    unordered_map<int, int>::iterator it = hiAncestor.begin();

    for (; it != hiAncestor.end(); ++it) {
        descendants[it->second].push_back(it->first);
    }

    unordered_map<int, vector<int> >::iterator dit = descendants.begin();
    for (; dit != descendants.end(); ++dit) {
        MetaNodePtr paramLHS;
        vector<MetaNodePtr> paramRHS;
        int anc = dit->first;
        const vector<int> &dList = dit->second;
        bool fromLHS = false;
        unordered_map<int, MetaNodePtr>::iterator mit;
        if ( (mit = lhsMap.find(anc)) != lhsMap.end() ) {
            paramLHS = mit->second;
            fromLHS = true;
            lhsMap.erase(mit);
        }
        else if ( (mit = rhsMap.find(anc)) != rhsMap.end() ) {
            paramLHS = mit->second;
            rhsMap.erase(mit);
        }
        else {
            // Problem if it gets here
            assert(false);
        }
        BOOST_FOREACH(int i, dList) {
            if ( fromLHS && (mit = rhsMap.find(i)) != rhsMap.end() ) {
                paramRHS.push_back(mit->second);
                rhsMap.erase(mit);
            }
            else if ( (mit = lhsMap.find(i)) != lhsMap.end() ) {
                paramRHS.push_back(mit->second);
                lhsMap.erase(mit);
            }
        }
        ret.push_back(make_pair<MetaNodePtr, vector<MetaNodePtr> >(paramLHS, paramRHS));
    }
    return ret;

}


// Public functions below here

MetaNodePtr NodeManager::CreateMetaNode(const Scope &var,
        const vector<ANDNodePtr> &ch, double weight) {
    MetaNodePtr temp(new MetaNode(var, ch));
    temp->SetWeight(weight);
    UniqueTable::iterator it = ut.find(temp);
    if (it != ut.end()) {
        return *it;
    }
    else {
        ut.insert(temp);
        return temp;
    }
}

MetaNodePtr NodeManager::CreateMetaNode(int varid, unsigned int card,
        const vector<ANDNodePtr> &ch, double weight) {
    Scope var;
    var.AddVar(varid, card);
    return CreateMetaNode(var, ch, weight);
}

MetaNodePtr NodeManager::CreateMetaNode(const Scope &vars,
        const vector<double> &vals, double weight) {
    assert(vars.GetCard() == vals.size());
    int rootVarID = vars.GetOrdering().front();
    unsigned int card = vars.GetVarCard(rootVarID);
    Scope rootVar;
    rootVar.AddVar(rootVarID, card);
    vector<ANDNodePtr> children;

    // Need to split the values vector if we are not at a leaf
    if (card != vals.size()) {
        Scope v(vars);
        Scope restVars = v - rootVar;

        // Dummy variable case
        if (card == 1) {
            vector<MetaNodePtr> ANDch;
            ANDch.push_back(CreateMetaNode(restVars, vals, weight));
            ANDNodePtr newNode(new MetaNode::ANDNode(1.0, ANDch));
            children.push_back(newNode);
        }
        // General case
        else {
            vector<vector<double> > valParts = SplitVector(vals, card);
            for (unsigned int i = 0; i < card; i++) {
                vector<MetaNodePtr> ANDch;
                ANDch.push_back(CreateMetaNode(restVars, valParts[i], weight));
                ANDNodePtr newNode(new MetaNode::ANDNode(1.0, ANDch));
                children.push_back(newNode);
            }
        }
    }
    // Otherwise we are at the leaves
    else {
        for (unsigned int i = 0; i < card; i++) {
            const MetaNodePtr &terminal = ((vals[i] == 0) ? MetaNode::GetZero()
                    : MetaNode::GetOne());
            vector<MetaNodePtr> ANDch;
            ANDch.push_back(terminal);
            ANDNodePtr newNode(new MetaNode::ANDNode(vals[i], ANDch));
            children.push_back(newNode);
        }
    }
    MetaNodePtr ret(CreateMetaNode(rootVar, children, weight));
    return ret;
}

vector<MetaNodePtr> NodeManager::FullReduce(MetaNodePtr node, double &w) {
    // terminal check
    if (node.get() == MetaNode::GetZero().get() || node.get()
            == MetaNode::GetOne().get()) {
        return vector<MetaNodePtr> (1, node);
    }
    bool redundant = true;
    const vector<ANDNodePtr> &ch = node->GetChildren();
    ANDNodePtr temp = ch[0];
    for (unsigned int i = 1; i < ch.size(); ++i) {
        if (temp != ch[i]) {
            redundant = false;
            break;
        }
    }

    if (redundant) {
        ut.erase(node);
        vector<MetaNodePtr> andCh = temp->GetChildren();
        vector<MetaNodePtr> ret;
        for (unsigned int i = 0; i < andCh.size(); ++i) {
            vector<MetaNodePtr> reduceSet = FullReduce(andCh[i], w);
            BOOST_FOREACH(MetaNodePtr mn, reduceSet) {
                if ( !ret.empty() ) {
                    if ( ret.back().get() == MetaNode::GetOne().get() ) {
                        ret.pop_back();
                    }
                    else if ( mn.get() == MetaNode::GetOne().get() ) {
                        continue;
                    }
                }
                ret.push_back(mn);
            }
        }
        BOOST_FOREACH(MetaNodePtr i, ret) {
            ut.insert(i);
        }
        w = temp->GetWeight();
        return ret;
    }
    else {
        vector<ANDNodePtr> newMetaCh;
        for (unsigned int i = 0; i < ch.size(); ++i) {
            vector<MetaNodePtr> andCh = ch[i]->GetChildren();
            vector<MetaNodePtr> newandCh;
            for (unsigned int j = 0; j < andCh.size(); ++j) {
                vector<MetaNodePtr> reduceSet = FullReduce(andCh[j], w);
                BOOST_FOREACH(MetaNodePtr mn, reduceSet) {
                    if ( !newandCh.empty() ) {
                        if ( newandCh.back().get() == MetaNode::GetOne().get() ) {
                            newandCh.pop_back();
                        }
                        else if ( mn.get() == MetaNode::GetOne().get() ) {
                            continue;
                        }
                    }
                    newandCh.push_back(mn);
                }
            }
            ANDNodePtr
                    rAND(new MetaNode::ANDNode(w * ch[i]->GetWeight(), newandCh));
            newMetaCh.push_back(rAND);
            w = 1.0;
        }
        node->SetChildren(newMetaCh);
        ut.insert(node);
        return vector<MetaNodePtr>(1, node);
    }

}

MetaNodePtr NodeManager::Apply(MetaNodePtr lhs,
        const vector<MetaNodePtr> &rhs,
        Operator op,
        const DirectedGraph &embeddedPT,
        double w) {
    Operation ocEntry(op, lhs, rhs);
    OperationCache::iterator ocit = opCache.find(ocEntry);
    if ( ocit != opCache.end() ) {
        //Found result in cache
        return ocit->second;
    }

    // Base cases
    switch(op) {
        case PROD:
            if ( lhs.get() == MetaNode::GetZero().get() ) {
                return MetaNode::GetZero();
            }
            else if ( rhs.size() == 0 ) {
                return lhs;
            }
            else {
                for (unsigned int i = 0; i < rhs.size(); ++i) {
                    if ( rhs[i].get() == MetaNode::GetZero().get() ) {
                        return MetaNode::GetZero();
                    }
                }
            }
            if ( lhs.get() == MetaNode::GetOne().get() ) {
                return MetaNode::GetOne();
            }
            break;
        case SUM:
            if ( rhs.size() == 0 ) {
                return lhs;
            }
            if ( lhs.get() == MetaNode::GetOne().get() ) {
                return MetaNode::GetOne();
            }
            break;
        default:
            assert(false);
    }

    int varid = lhs->GetVarID();
    int card = lhs->GetCard();

    vector<ANDNodePtr> children;

    // For each value of lhs
    for (int k = 0; k < card; ++k) {
        // Get original weight
        vector<MetaNodePtr> newChildren;
        double weight = w;
        weight *= lhs->GetChildren()[k]->GetWeight() * lhs->GetWeight();
        vector<MetaNodePtr> lhsChildren =
                lhs->GetChildren()[k]->GetChildren();
        vector<MetaNodePtr> tempChildren;

        if ( rhs.size() == 1 && varid == rhs[0]->GetVarID() ) {
            // Same variable, single roots case
            tempChildren = rhs[0]->GetChildren()[k]->GetChildren();
            switch(op) {
                case PROD:
                    weight *= rhs[0]->GetChildren()[k]->GetWeight() *
	                    rhs[0]->GetWeight();
                    break;
                case SUM:
                    weight += rhs[0]->GetChildren()[k]->GetWeight() *
	                    rhs[0]->GetWeight();

                    // push weights down
                    lhsChildren = ReweighNodes(lhsChildren,
                            lhs->GetChildren()[k]->GetWeight() * lhs->GetWeight());
                    tempChildren = ReweighNodes(tempChildren,
                            rhs[0]->GetChildren()[k]->GetWeight() * rhs[0]->GetWeight());
                    break;
                default:
                    assert(false);
            }
        }
        else {
            // Not the same variable, prepare to push rhs down
            tempChildren = rhs;

            // Still need to reweight lhs regardless
            if (op == SUM) {
//                tempChildren.clear();
                lhsChildren = ReweighNodes(lhsChildren,
                        lhs->GetChildren()[k]->GetWeight() * lhs->GetWeight());
                /*
                BOOST_FOREACH(MetaNodePtr rmn, rhs) {
                    vector<MetaNodePtr> tCh = rmn->GetChildren()[k]->GetChildren();
                    tCh = ReweighNodes(tCh)
                }
                */
            }
        }

        /*

        // Group nodes into parameter sets for recursive applys
        unordered_set<int> lhsSet;
        unordered_set<int> tempSet;

        // Create maps from the ancestor node to varids.
        unordered_map<int, vector<int> > ancestorMap;

        // Create map from integers to MetaNodes for quick access later
        unordered_map<int, MetaNodePtr> metaNodeMapLeft;
        unordered_map<int, MetaNodePtr> metaNodeMapRight;

        for (unsigned int i = 0; i < lhsChildren.size(); ++i) {
                lhsSet.insert(lhsChildren[i]->GetVarID());
                metaNodeMapLeft.insert(make_pair<int, MetaNodePtr>(
                        lhsChildren[i]->GetVarID(), lhsChildren[i]));
        }
        for (unsigned int i = 0; i < tempChildren.size(); ++i) {
            tempSet.insert(tempChildren[i]->GetVarID());
            metaNodeMapRight.insert(make_pair<int, MetaNodePtr>(
                    tempChildren[i]->GetVarID(), tempChildren[i]));
        }

        // Find highest ancestor within the other set
        unordered_set<int>::iterator lhsIt = lhsSet.begin();
        for (; lhsIt != lhsSet.end(); ++lhsIt) {
            int chvarid = *lhsIt;

            // No need to traverse if determined to be an ancestor
            if (ancestorMap.find(chvarid) != ancestorMap.end()) {
                continue;
            }

            if (chvarid == -1) {
                ancestorMap[-1].push_back(-1);
                continue;
            }

            // Check pseudo tree
            DInEdge ei, ei_end;
            tie(ei, ei_end) = in_edges(chvarid, embeddedPT);

            int highestAncestor = chvarid;
            int count = 0; // Debugging use
            while (ei != ei_end) {
                assert(++count <= 1);
                int parent = source(*ei, embeddedPT);
                if ( lhsSet.find(parent) != tempSet.end() || tempSet.find(parent) != tempSet.end() ) {
                    highestAncestor = parent;
                    tie(ei, ei_end) = in_edges(parent, embeddedPT);
                    count = 0;
                }
                ++ei;
            }
            ancestorMap[highestAncestor].push_back(chvarid);
        }

        // Repeat from other side
        unordered_set<int>::iterator tempIt = tempSet.begin();
        for (; tempIt != tempSet.end(); ++tempIt) {
            int chvarid = *tempIt;

            // No need to traverse if determined to be an ancestor
            if (ancestorMap.find(chvarid) != ancestorMap.end()) {
                continue;
            }

            if (chvarid == -1) {
                ancestorMap[-1].push_back(-1);
                continue;
            }

            // Check pseudo tree
            DInEdge ei, ei_end;
            tie(ei, ei_end) = in_edges(chvarid, embeddedPT);
            int highestAncestor = chvarid;
            while (ei != ei_end) {
                int parent = source(*ei, embeddedPT);
                if ( lhsSet.find(parent) != lhsSet.end() || tempSet.find(parent) != tempSet.end() ) {
                    highestAncestor = parent;
                    tie(ei, ei_end) = in_edges(parent, embeddedPT);
                }
                ++ei;
            }
            ancestorMap[highestAncestor].push_back(chvarid);
        }

        vector< pair<MetaNodePtr, vector<MetaNodePtr> > > paramSets;
        unordered_map<int, vector<int> >::iterator ait = ancestorMap.begin();

        ait = ancestorMap.begin();
        unordered_map<int,MetaNodePtr>::iterator mit;
        for (; ait != ancestorMap.end(); ++ait) {
            vector<MetaNodePtr> paramRHS;
            const vector<int> &descendants = ait->second;

            // Find a suitable left hand parameter
            mit = metaNodeMapLeft.find(ait->first);
            MetaNodePtr temp;
            if (mit == metaNodeMapLeft.end()) {
                mit = metaNodeMapRight.find(ait->first);
                temp = mit->second;
                metaNodeMapRight.erase(mit);
            }
            else {
                temp = mit->second;
                metaNodeMapLeft.erase(mit);
            }

            // Build up right hand parameter
            for (unsigned int i = 0; i < descendants.size(); ++i) {
                mit = metaNodeMapRight.find(descendants[i]);
                if (mit == metaNodeMapRight.end()) {
                    mit = metaNodeMapLeft.find(descendants[i]);
                    if (mit != metaNodeMapLeft.end()) {
                        paramRHS.push_back(mit->second);
                        metaNodeMapLeft.erase(mit);
                    }
                }
                else {
                    paramRHS.push_back(mit->second);
                    metaNodeMapRight.erase(mit);
                }
//                if (ait->first != descendants[i]) {
//                }
            }
            paramSets.push_back(make_pair<MetaNodePtr, vector<MetaNodePtr> >(
                    temp, paramRHS));
        }
        */
        vector<ApplyParamSet> paramSets = GetParamSets(embeddedPT, lhsChildren, tempChildren);


        cout << endl << "Start param sets" << endl;
        // For each parameter set
        for (unsigned int i = 0; i < paramSets.size(); ++i) {
            cout << "lhs: ";
            cout << paramSets[i].first->GetVarID() << endl;
//            paramSets[i].first->Save(cout); cout << endl << endl;
            cout << "rhs:";
            BOOST_FOREACH(MetaNodePtr mn, paramSets[i].second) {
                cout << " " << mn->GetVarID();
//              cout << endl;  mn->Save(cout); cout << endl;
            }
            cout << endl;
            MetaNodePtr subDD = Apply(paramSets[i].first, paramSets[i].second, op, embeddedPT, w);
            if ( subDD.get() == MetaNode::GetZero().get() ) {
                newChildren.push_back(MetaNode::GetZero());
                break;
            }
            else {
                if( !newChildren.empty() && newChildren.back().get() == MetaNode::GetOne().get() ) {
                    newChildren.pop_back();
                }
                else if ( !newChildren.empty() && subDD.get() == MetaNode::GetOne().get() ) {
                    continue;
                }
                newChildren.push_back(subDD);
            }
            if (op == SUM &&
                    subDD.get() != MetaNode::GetZero().get() &&
                    subDD.get() != MetaNode::GetOne().get()) {
                cout << "Not at leaves, weight was "<< weight << endl;
                weight = 1;
            }
        }
        /*
        cout << "value=" << k << endl;
        cout << "Old weight = " << lhs->GetChildren()[k]->GetWeight() << endl;
        cout << "New weight = " << weight << endl;
        */

        ANDNodePtr newNode(new MetaNode::ANDNode(weight, newChildren));
        children.push_back(newNode);
    }
    // Redundancy can be resolved outside
    Scope var;
    var.AddVar(varid, card);
    MetaNodePtr u = CreateMetaNode(var, children);
    Operation entryKey(op, lhs, rhs);
    opCache.insert(make_pair<Operation, MetaNodePtr>(entryKey, u));
    return u;
}

// Sets each AND node of variables to marginalize to be the result of summing
// the respective MetaNode children of each AND node. Redundancy can be
// resolved outside.
MetaNodePtr NodeManager::Marginalize(MetaNodePtr root, const Scope &s,
        const DirectedGraph &embeddedpt) {
    if (root.get() == MetaNode::GetZero().get()) {
        return MetaNode::GetZero();
    }
    else if (root.get() == MetaNode::GetOne().get()) {
        return MetaNode::GetOne();
    }
    int varid = root->GetVarID();
    int card = root->GetCard();

    // Marginalize each subgraph
    const vector<ANDNodePtr> &andNodes = root->GetChildren();
    vector<ANDNodePtr> newANDNodes;
    BOOST_FOREACH(ANDNodePtr i, andNodes) {
        const vector<MetaNodePtr> &metaNodes = i->GetChildren();
        vector<MetaNodePtr> newMetaNodes;
        BOOST_FOREACH(MetaNodePtr j, metaNodes) {
            MetaNodePtr newMetaNode = Marginalize(j, s, embeddedpt);
            if ( !newMetaNodes.empty() && newMetaNodes.back().get() == MetaNode::GetOne().get() ) {
                newMetaNodes.pop_back();
            }
            else if ( !newMetaNodes.empty() && newMetaNode.get() == MetaNode::GetOne().get() ) {
//                cout << "Skipped adding a terminal" << endl;
                continue;
            }
//            cout << "Adding node " << newMetaNode.get() << " in marginalize, (to varid = "
//                    << varid << endl;
            newMetaNodes.push_back(newMetaNode);
        }
        ANDNodePtr newANDNode(new MetaNode::ANDNode(i->GetWeight(), newMetaNodes));
        newANDNodes.push_back(newANDNode);
    }

    // If the root is to be marginalized
    if (s.VarExists(varid)) {
        // Make a copy for the first value
        const vector<MetaNodePtr> &firstMetaNodes = newANDNodes[0]->GetChildren();
        double weight = 0;

        // Propagate the AND weight downward
        vector<MetaNodePtr> newMetaNodes = ReweighNodes(firstMetaNodes, newANDNodes[0]->GetWeight());
        weight += newANDNodes[0]->GetWeight();

        for (unsigned int i = 1; i < newANDNodes.size(); ++i) {
            const vector<MetaNodePtr> &curMetaNodes = newANDNodes[i]->GetChildren();
            vector<MetaNodePtr> tempMetaNodes = ReweighNodes(curMetaNodes, newANDNodes[i]->GetWeight());
            weight += newANDNodes[i]->GetWeight();

            // "+=" on newMetaNodes
            vector<ApplyParamSet> paramSet = GetParamSets(embeddedpt, newMetaNodes, tempMetaNodes);
            cout << "Marginalize paramsets" << endl;
            BOOST_FOREACH(ApplyParamSet aps, paramSet) {
                cout << "lhs: " << aps.first->GetVarID() << "(" << aps.first.get() << "), ";
                cout << "rhs:";
                BOOST_FOREACH(MetaNodePtr rn, aps.second) {
                    cout << " " << rn->GetVarID() << "(" << rn.get() << ") ";
                }
                cout << endl;
            }
            newMetaNodes.clear();
            for (unsigned int j = 0; j < paramSet.size(); ++j) {
                MetaNodePtr newMeta = Apply(paramSet[j].first, paramSet[j].second, SUM, embeddedpt);
                cout << "Original input DDs" << endl;
                paramSet[j].first->RecursivePrint(cout); cout << endl;
                paramSet[j].second[0]->RecursivePrint(cout); cout << endl;
                cout << "Apply results" << endl;
                cout << "lhs: " << paramSet[j].first->GetVarID() << "(" << paramSet[j].first.get() << "), ";
                cout << "rhs:";
                BOOST_FOREACH(MetaNodePtr rn, paramSet[j].second) {
                    cout << " " << rn->GetVarID() << "(" << rn.get() << ") ";
                }
                cout << endl;
                newMeta->RecursivePrint(cout); cout << endl << endl;
                newMetaNodes.push_back(newMeta);
            }
        }
        newANDNodes.clear();
        if (newMetaNodes.size() > 1 || newMetaNodes[0].get() != MetaNode::GetOne().get())
            weight = 1;
        ANDNodePtr newAND(new MetaNode::ANDNode(weight, newMetaNodes));
        for (unsigned int i = 0; i < andNodes.size(); ++i) {
            newANDNodes.push_back(newAND);
        }
    }
    Scope var;
    var.AddVar(varid, card);
    MetaNodePtr ret = CreateMetaNode(var, newANDNodes, root->GetWeight());
    return ret;
}

MetaNodePtr NodeManager::Condition(MetaNodePtr root, const Assignment &cond) {
    if (root.get() == MetaNode::GetZero().get() ||
            root.get() == MetaNode::GetOne().get()) {
        return root;
    }

    vector<ANDNodePtr> newANDNodes;
    const vector<ANDNodePtr> &rootCh = root->GetChildren();
    int val;
    if ( (val = cond.GetVal(root->GetVarID())) != ERROR_VAL ) {
        vector<MetaNodePtr> newMeta;
        BOOST_FOREACH(MetaNodePtr j, rootCh[val]->GetChildren()) {
            newMeta.push_back(Condition(j, cond));
        }
        ANDNodePtr newAND(new MetaNode::ANDNode(rootCh[val]->GetWeight(), newMeta));
        newANDNodes.push_back(newAND);
    }
    else {
        BOOST_FOREACH(ANDNodePtr i, root->GetChildren()) {
            vector<MetaNodePtr> newMeta;
            BOOST_FOREACH(MetaNodePtr j, i->GetChildren()) {
                newMeta.push_back(Condition(j, cond));
            }
            ANDNodePtr newAND(new MetaNode::ANDNode(i->GetWeight(), newMeta));
            newANDNodes.push_back(newAND);
        }
    }
    MetaNodePtr ret = CreateMetaNode(root->GetVarID(), root->GetCard(), newANDNodes);
    return ret;
}

MetaNodePtr NodeManager::Normalize(MetaNodePtr root) {
    MetaNodePtr ret = NormalizeHelper(root);
    ut.insert(ret);
    return ret;
}

MetaNodePtr NodeManager::NormalizeHelper(MetaNodePtr root) {
    if (root.get() == MetaNode::GetZero().get() ||
            root.get() == MetaNode::GetOne().get()) {
        return root;
    }
    double normConstant = 0;
    vector<ANDNodePtr> children;
    BOOST_FOREACH(ANDNodePtr i, root->GetChildren()) {
        const vector<MetaNodePtr> &achildren = i->GetChildren();
        vector<MetaNodePtr> newANDChildren;
        double w = i->GetWeight();
        BOOST_FOREACH(MetaNodePtr j, achildren) {
            MetaNodePtr newMeta = NormalizeHelper(j);
            w *= newMeta->GetWeight();
            newMeta->SetWeight(1);
            newANDChildren.push_back(newMeta);
            ut.insert(newMeta);
        }
        normConstant += w;
        ANDNodePtr newANDNode(new MetaNode::ANDNode(w, newANDChildren));
        children.push_back(newANDNode);
    }

    BOOST_FOREACH(ANDNodePtr i, children) {
        i->SetWeight(i->GetWeight() / normConstant);
    }
    double newMetaWeight = root->GetWeight() * normConstant;
    MetaNodePtr ret(new MetaNode(root->GetVarID(), root->GetCard(),
            children));
    ret->SetWeight(newMetaWeight);
    return ret;
}

unsigned int NodeManager::GetNumberOfNodes() const {
    return ut.size();
}

void NodeManager::PrintUniqueTable(ostream &out) const {
    BOOST_FOREACH (MetaNodePtr i, ut) {
        i->Save(out); out << endl;
    }
}

bool NodeManager::initialized = false;
NodeManager *NodeManager::singleton = NULL;

} // end of aomdd namespace
