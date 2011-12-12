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

#define MAX(a, b) (a > b ? a : b)

namespace aomdd {
using namespace std;

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

vector<ApplyParamSet> NodeManager::GetParamSets(const DirectedGraph &tree,
        const vector<MetaNodePtr> &lhs, const vector<MetaNodePtr> &rhs) const {
    vector<ApplyParamSet> ret;
    // First check if rhs is terminal and lhs is size 1
    if (lhs.size() == 1 && rhs.size() == 1 && rhs[0]->IsTerminal()) {
        ret.push_back(make_pair<MetaNodePtr, vector<MetaNodePtr> >(lhs[0], rhs));
        return ret;
    }

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
        if (varid < 0) continue;

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
        if (varid < 0) continue;

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

WeightedMetaNodeList NodeManager::CreateMetaNode(const Scope &var,
        const vector<ANDNodePtr> &ch) {
    MetaNodePtr newNode(new MetaNode(var, ch));
    WeightedMetaNodeList temp = SingleLevelFullReduce(newNode);
    if (temp.first.size() > 1 || temp.first[0].get() != newNode.get()) {
        return temp;
    }
    temp.second *= temp.first[0]->Normalize();
    return LookupUT(temp);
    /*
    UniqueTable::iterator it = ut.find(temp.first[0]);
    if (it != ut.end()) {
        return WeightedMetaNodeList(MetaNodeList(1, *it), temp.second);
    }
    else {
        ut.insert(temp.first[0]);
        return temp;
    }
    */
}

WeightedMetaNodeList NodeManager::CreateMetaNode(int varid, unsigned int card,
        const vector<ANDNodePtr> &ch) {
    Scope var;
    var.AddVar(varid, card);
    return CreateMetaNode(var, ch);
}

WeightedMetaNodeList NodeManager::CreateMetaNode(const Scope &vars,
        const vector<double> &vals) {
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

        /*
        // Dummy variable case
        if (card == 1) {
            vector<MetaNodePtr> ANDch;
            ANDch.push_back(CreateMetaNode(restVars, vals, weight));
            ANDNodePtr newNode(new MetaNode::ANDNode(1.0, ANDch));
            children.push_back(newNode);
        }
        // General case
         */
//        else {
        vector<vector<double> > valParts = SplitVector(vals, card);
        for (unsigned int i = 0; i < card; i++) {
            vector<MetaNodePtr> ANDch;
            WeightedMetaNodeList l = CreateMetaNode(restVars, valParts[i]);
            ANDNodePtr newNode(new MetaNode::ANDNode(l.second, l.first));
            children.push_back(newNode);
        }
//        }
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
    return CreateMetaNode(rootVar, children);
}

WeightedMetaNodeList NodeManager::SingleLevelFullReduce(MetaNodePtr node) {
    // terminal check
    if (node.get() == MetaNode::GetZero().get() ||
            node.get() == MetaNode::GetOne().get()) {
        double weight = node.get() == MetaNode::GetZero().get() ? 0.0 : 1.0;
        return WeightedMetaNodeList(MetaNodeList(1, node), weight);
    }

    const vector<ANDNodePtr> &ch = node->GetChildren();

    bool redundant = true;
    ANDNodePtr temp = ch[0];
    for (unsigned int i = 1; i < ch.size(); ++i) {
        if (temp != ch[i]) {
            redundant = false;
            break;
        }
    }

    if ( redundant ) {
        return WeightedMetaNodeList(ch[0]->GetChildren(), ch[0]->GetWeight());
    }
    else {
        return WeightedMetaNodeList(MetaNodeList(1, node), 1.0);
    }
}

/*j
vector<MetaNodePtr> NodeManager::FullReduce(MetaNodePtr node, double &w, bool isRoot) {
    // terminal check
    if (node.get() == MetaNode::GetZero().get() || node.get()
            == MetaNode::GetOne().get()) {
        return vector<MetaNodePtr> (1, node);
    }

    const vector<ANDNodePtr> &ch = node->GetChildren();
    vector<ANDNodePtr> newCh;
    for (unsigned int i = 0; i < ch.size(); ++i) {
        const vector<MetaNodePtr> &andCh = ch[i]->GetChildren();
        vector<MetaNodePtr> reduced;
        for (unsigned int j = 0; j < andCh.size(); ++j) {
            double wr = 1.0;
            vector<MetaNodePtr> temp = FullReduce(andCh[j], wr);
            BOOST_FOREACH(MetaNodePtr mn, temp) {
                if ( mn.get() == MetaNode::GetZero().get() ) {
                    wr = 0;
                    reduced.clear();
                    reduced.push_back(mn);
                    break;
                }
                if ( !reduced.empty() ) {
                    if ( reduced.back().get() == MetaNode::GetOne().get() ) {
                        reduced.pop_back();
                    }
                    else if ( mn.get() == MetaNode::GetOne().get() ) {
                        continue;
                    }
                }
                reduced.push_back(mn);
            }
            w *= wr;
            if (w == 0) break;
        }
        ANDNodePtr rAND(new MetaNode::ANDNode(w * ch[i]->GetWeight(), reduced));
        newCh.push_back(rAND);
        w = 1.0;
    }

    bool redundant = true;
    ANDNodePtr temp = newCh[0];
    if (newCh.size() == 1 && isRoot) redundant = false;
    for (unsigned int i = 1; i < newCh.size(); ++i) {
        if (temp != newCh[i]) {
            redundant = false;
            break;
        }
    }

    if ( redundant ) {
        w *= newCh[0]->GetWeight();
        return newCh[0]->GetChildren();
    }
    else {
        MetaNodePtr ret = CreateMetaNode(node->GetVarID(), node->GetCard(), newCh);
        return vector<MetaNodePtr>(1, ret);
    }
}

MetaNodePtr NodeManager::FullReduce(MetaNodePtr root) {
    double w = 1.0;

    Operation ocEntry(REDUCE, root);
    OperationCache::iterator ocit = opCache.find(ocEntry);
    if ( ocit != opCache.end() ) {
        //Found result in cache
        return ocit->second;
    }

    vector<MetaNodePtr> nodes = FullReduce(root, w, true);

    // Reduced to a single root and weight was not reweighted
    if (nodes.size() == 1 && fabs(w - 1.0) < TOLERANCE) {
        return nodes[0];
    }

    int varid = root->GetVarID();
    ANDNodePtr newAND(new MetaNode::ANDNode(w, nodes));
    MetaNodePtr newMeta = CreateMetaNode(varid, 1, vector<ANDNodePtr>(1, newAND));
    Operation entryKey(REDUCE, root);
    opCache.insert(make_pair<Operation, MetaNodePtr>(entryKey, newMeta));
    return newMeta;
}
*/

WeightedMetaNodeList NodeManager::Apply(MetaNodePtr lhs,
        const MetaNodeList &rhs,
        Operator op,
        const DirectedGraph &embeddedPT) {
    int varid = lhs->GetVarID();
    int card = lhs->GetCard();

    Operation ocEntry(op, lhs, rhs);
    OperationCache::iterator ocit = opCache.find(ocEntry);

    if ( ocit != opCache.end() ) {
        return ocit->second;
    }

    // Base cases
    switch(op) {
        case PROD:
            // If its a terminal the rhs must be same terminals
            if ( rhs.size() == 0 || lhs->IsTerminal() ) {
                return WeightedMetaNodeList(MetaNodeList(1, lhs), 1.0);
            }
            // Look for any zeros on the rhs. Result is zero if found
            else {
                for (unsigned int i = 0; i < rhs.size(); ++i) {
                    if ( rhs[i].get() == MetaNode::GetZero().get() ) {
                        return WeightedMetaNodeList(MetaNodeList(1, MetaNode::GetZero()), 0.0);
                    }
                }
            }

            /*
            if ( lhs.get() == MetaNode::GetOne().get() ) {
                return MetaNode::GetOne();
            }
            */
            break;
        case SUM:
            if ( rhs.size() == 0 || lhs->IsTerminal() ) {
                return WeightedMetaNodeList(MetaNodeList(1, lhs), 1.0);
            }
            break;
        case MAX:
            if ( rhs.size() == 0 || lhs->IsTerminal() ) {
                return WeightedMetaNodeList(MetaNodeList(1, lhs), 1.0);
            }
            break;
        default:
            assert(false);
            break;
    }

    // Should have detected terminals
    assert(varid >= 0);


    vector<ANDNodePtr> children;

    // For each value of lhs
    for (int k = 0; k < card; ++k) {
        // Get original weight
        vector<MetaNodePtr> newChildren;
        double weight = 1.0;
        weight *= lhs->GetChildren()[k]->GetWeight();
        vector<ApplyParamSet> paramSets;
        const vector<MetaNodePtr> &paramLHS =
                lhs->GetChildren()[k]->GetChildren();

        if ( rhs.size() == 1 && varid == rhs[0]->GetVarID() ) {
            // Same variable, single roots case
            double rhsWeight = rhs[0]->GetChildren()[k]->GetWeight();
            switch(op) {
                case PROD:
                    weight *= rhsWeight;
                    break;
                case SUM:
                    weight += rhsWeight;
                    break;
                case MAX:
                    weight = MAX(weight, rhsWeight);
                    break;
                default:
                    assert(false);
                    break;
            }
            paramSets = GetParamSets(embeddedPT, paramLHS, rhs[0]->GetChildren()[k]->GetChildren());
        }
        else {
            // Not the same variable, prepare to push rhs down
            paramSets = GetParamSets(embeddedPT, paramLHS, rhs);
        }

        bool terminalOnly = true;
        bool earlyTerminate = false;
        if (weight == 0) {
            newChildren.push_back(MetaNode::GetZero());
        }

        else {
            // For each parameter set
            BOOST_FOREACH(ApplyParamSet aps, paramSets) {
                WeightedMetaNodeList subDD = Apply(aps.first, aps.second, op, embeddedPT);
                /*
            cout << "lhs: " << aps.first->GetVarID() << ", ";
            cout << "rhs:";
            BOOST_FOREACH(MetaNodePtr r, aps.second) {
                cout << " " << r->GetVarID();
            }
            cout << endl;

            cout << "SubDD:" << "(" << varid << "," << k << ")" << endl;
            BOOST_FOREACH(MetaNodePtr msub, subDD.first) {
                msub->RecursivePrint(cout); cout << endl;
            }
                 */

                // Note: would have to modify for other operators
                /*
                cout << "weight before multiplying in subDD weight:" << weight << endl;
                cout << "subDD list length:" << subDD.first.size() << endl;
                cout << "weight after multiplying in subDD weight:" << weight << endl;
                subDD.first[0]->RecursivePrint(cout); cout << endl;
                */
                weight *= subDD.second;
                BOOST_FOREACH(MetaNodePtr m, subDD.first) {
                    if (op == PROD && m.get() == MetaNode::GetZero().get()) {
//                        cout << "Found a zero terminal" << endl;
                        newChildren.clear();
                        newChildren.push_back(m);
                        earlyTerminate = true;
                        break;
                    }
                    if (!m->IsTerminal()) {
                        if (terminalOnly) {
                            newChildren.clear();
                            terminalOnly = false;
                        }
                        newChildren.push_back(m);
                    }
                    else if (newChildren.empty()) {
                        newChildren.push_back(m);
                    }
                }
                if (earlyTerminate) break;
            }
        }

        if (weight == 0) {
            assert(newChildren.size() == 1);
            assert(newChildren[0].get() == MetaNode::GetZero().get());
        }
        ANDNodePtr newNode(new MetaNode::ANDNode(weight, newChildren));
        children.push_back(newNode);
    }
    Scope var;
    var.AddVar(varid, card);
    WeightedMetaNodeList u = CreateMetaNode(var, children);

    opCache.insert(make_pair<Operation, WeightedMetaNodeList>(ocEntry, u));
    opCacheMemUsage += (ocEntry.MemUsage() + sizeof(u) + (u.first.size() * sizeof(MetaNodePtr))) / MB_PER_BYTE;

    // Purge if op cache is too large
    if (opCacheMemUsage > OCMBLimit) {
        NodeManager::GetNodeManager()->PurgeOpCache();
    }

    if (opCacheMemUsage > maxOpCacheMemUsage) maxOpCacheMemUsage = opCacheMemUsage;
    /*
    cout << "Created cache entry" << endl;
    cout << "keys:";
    BOOST_FOREACH(size_t k, entryKey.GetParamSet()) {
        cout << " " << (void*)k;
    }
    cout << endl << "res:" << u.get() << endl;
    */

    if (utMemUsage > MBLimit) {
        cout << "Unique Table reached memory limit." << endl;
        exit(0);
    }

    return u;
}

// Sets each AND node of variables to marginalize to be the result of summing
// the respective MetaNode children of each AND node.

// Current bug: have to check for the case where the node is not present,
// but is a child of a node that is being marginalized.
WeightedMetaNodeList NodeManager::Marginalize(MetaNodePtr root, const Scope &s,
        const DirectedGraph &elimChain, bool &sumOpPerformed) {
    if (root.get() == MetaNode::GetZero().get()) {
        return WeightedMetaNodeList(MetaNodeList(1, MetaNode::GetZero()), 0.0);
    }
    else if (root.get() == MetaNode::GetOne().get()) {
        return WeightedMetaNodeList(MetaNodeList(1, MetaNode::GetOne()), 1.0);
    }
    int varid = root->GetVarID();
    int card = root->GetCard();
    int elimvar = s.GetOrdering().front();

    Operation ocEntry(MARGINALIZE, root, elimvar);
    OperationCache::iterator ocit = opCache.find(ocEntry);
    if ( ocit != opCache.end() ) {
        //Found result in cache
        return ocit->second;
    }

    // Marginalize each subgraph
    const vector<ANDNodePtr> &andNodes = root->GetChildren();
    vector<ANDNodePtr> newANDNodes;
    BOOST_FOREACH(ANDNodePtr i, andNodes) {
        const vector<MetaNodePtr> &metaNodes = i->GetChildren();
        vector<MetaNodePtr> newMetaNodes;
        bool terminalOnly = true;
        double weight = 1.0;
        bool foundMargVar = false;

        DEdge ei, ei_end;

        vector<int> possibleNextMarg;
        tie(ei, ei_end) = out_edges(varid, elimChain);
        while (ei != ei_end) {
            int child = target(*ei, elimChain);
            possibleNextMarg.push_back(child);
            tie(ei, ei_end) = out_edges(child, elimChain);
        }

        BOOST_FOREACH(MetaNodePtr j, metaNodes) {
            BOOST_FOREACH(int k, possibleNextMarg) {
                if (j->GetVarID() == k) {
                    foundMargVar = true;
                }
            }
            if (!foundMargVar) {
                newMetaNodes.push_back(j);
                continue;
            }
            WeightedMetaNodeList newMetaNodeList = Marginalize(j, s, elimChain, sumOpPerformed);
            /*
            cout << "Input lhs: " << "(w="<< w << ", rhs size=" << paramSets[i].second.size() << ")"<< endl;
            paramSets[i].first->RecursivePrint(cout); cout << endl;
            cout << "SubDD:" << "(" << varid << "," << k << ")" << endl;
            subDD->RecursivePrint(cout); cout << endl;
            */

            // Note: would have to modify for other operators
            weight *= newMetaNodeList.second;

            BOOST_FOREACH(MetaNodePtr m, newMetaNodeList.first) {
                if (!m->IsTerminal()) {
                    if (terminalOnly) {
                        newMetaNodes.clear();
                        terminalOnly = false;
                    }
                    newMetaNodes.push_back(m);
                }
                else if (newMetaNodes.empty()) {
                    newMetaNodes.push_back(m);
                }
            }
        }
        if (varid != elimvar && !foundMargVar) {
            weight *= s.GetVarCard(elimvar);
        }
        ANDNodePtr newANDNode(new MetaNode::ANDNode(i->GetWeight() * weight, newMetaNodes));
        newANDNodes.push_back(newANDNode);
    }

    // If the root is to be marginalized
    if (s.VarExists(varid)) {
//        sumOpPerformed = true;
        // Assume node resides at bottom
        double weight = 0;
        for (unsigned int i = 0; i < newANDNodes.size(); ++i) {
            weight += newANDNodes[i]->GetWeight();
        }
        newANDNodes.clear();
        vector<MetaNodePtr> newMetaNodes;
        if (weight == 0) {
            newMetaNodes.push_back(MetaNode::GetZero());
        } else {
            newMetaNodes.push_back(MetaNode::GetOne());
        }
        ANDNodePtr newAND(new MetaNode::ANDNode(weight, newMetaNodes));
//        for (unsigned int i = 0; i < andNodes.size(); ++i) {
            newANDNodes.push_back(newAND);
//        }
    }
    Scope var;
    var.AddVar(varid, card);
    WeightedMetaNodeList ret = CreateMetaNode(var, newANDNodes);
    opCache.insert(make_pair<Operation, WeightedMetaNodeList>(ocEntry, ret));
    opCacheMemUsage += (ocEntry.MemUsage() + sizeof(ret) + (ret.first.size() * sizeof(MetaNodePtr))) / MB_PER_BYTE;

    // Purge if op cache is too large
    if (opCacheMemUsage > OCMBLimit) {
        NodeManager::GetNodeManager()->PurgeOpCache();
    }

    if (opCacheMemUsage > maxOpCacheMemUsage) maxOpCacheMemUsage = opCacheMemUsage;
    return ret;
}

// Sets each AND node of variables to marginalize to be the result of maximizing
// the respective MetaNode children of each AND node. Redundancy can be
// resolved outside.
WeightedMetaNodeList NodeManager::Maximize(MetaNodePtr root, const Scope &s,
        const DirectedGraph &embeddedpt) {
    if (root.get() == MetaNode::GetZero().get()) {
        return WeightedMetaNodeList(MetaNodeList(1, MetaNode::GetZero()), 0.0);
    }
    else if (root.get() == MetaNode::GetOne().get()) {
        return WeightedMetaNodeList(MetaNodeList(1, MetaNode::GetOne()), 1.0);
    }
    int varid = root->GetVarID();
    int card = root->GetCard();
    int elimvar = s.GetOrdering().front();

    Operation ocEntry(MAX, root, elimvar);
    OperationCache::iterator ocit = opCache.find(ocEntry);
    if ( ocit != opCache.end() ) {
        //Found result in cache
        /*
        cout << "operator: " << ocit->first.GetOperator() << endl;
        cout << "root: " << root.get() << endl;
        cout << "elimvar: " << elimvar << endl;
        cout << "Returning: " << ocit->second.get() << endl;
        */
        return ocit->second;
    }

    // Maximize each subgraph
    const vector<ANDNodePtr> &andNodes = root->GetChildren();
    vector<ANDNodePtr> newANDNodes;
    BOOST_FOREACH(ANDNodePtr i, andNodes) {
        const vector<MetaNodePtr> &metaNodes = i->GetChildren();
        vector<MetaNodePtr> newMetaNodes;
        bool terminalOnly = true;
        double weight = 1.0;
        BOOST_FOREACH(MetaNodePtr j, metaNodes) {
            WeightedMetaNodeList newMetaNodeList = Maximize(j, s, embeddedpt);
            /*
            cout << "Input lhs: " << "(w="<< w << ", rhs size=" << paramSets[i].second.size() << ")"<< endl;
            paramSets[i].first->RecursivePrint(cout); cout << endl;
            cout << "SubDD:" << "(" << varid << "," << k << ")" << endl;
            subDD->RecursivePrint(cout); cout << endl;
            */

            // Note: would have to modify for other operators
            weight *= newMetaNodeList.second;
            BOOST_FOREACH(MetaNodePtr m, newMetaNodeList.first) {
                if (!m->IsTerminal()) {
                    if (terminalOnly) {
                        newMetaNodes.clear();
                        terminalOnly = false;
                    }
                    newMetaNodes.push_back(m);
                }
                else if (newMetaNodes.empty()) {
                    newMetaNodes.push_back(m);
                }
            }
        }
        ANDNodePtr newANDNode(new MetaNode::ANDNode(i->GetWeight() * weight, newMetaNodes));
        newANDNodes.push_back(newANDNode);
    }

    // If the root is to be maximized
    if (s.VarExists(varid)) {
        // Assume node resides at bottom
        double weight = DOUBLE_MIN;
        for (unsigned int i = 0; i < newANDNodes.size(); ++i) {
            double temp = newANDNodes[i]->GetWeight();
            if (temp > weight) {
                weight = temp;
            }
        }
        newANDNodes.clear();
        vector<MetaNodePtr> newMetaNodes;
        if (weight == 0) {
            newMetaNodes.push_back(MetaNode::GetZero());
        } else {
            newMetaNodes.push_back(MetaNode::GetOne());
        }
        ANDNodePtr newAND(new MetaNode::ANDNode(weight, newMetaNodes));
        for (unsigned int i = 0; i < andNodes.size(); ++i) {
            newANDNodes.push_back(newAND);
        }
    }
    Scope var;
    var.AddVar(varid, card);
    WeightedMetaNodeList ret = CreateMetaNode(var, newANDNodes);
    opCache.insert(make_pair<Operation, WeightedMetaNodeList>(ocEntry, ret));
    opCacheMemUsage += (ocEntry.MemUsage() + sizeof(ret) + (ret.first.size() * sizeof(MetaNodePtr))) / MB_PER_BYTE;

    // Purge if op cache is too large
    if (opCacheMemUsage > OCMBLimit) {
        NodeManager::GetNodeManager()->PurgeOpCache();
    }

    if (opCacheMemUsage > maxOpCacheMemUsage) maxOpCacheMemUsage = opCacheMemUsage;
    /*
    cout << "Created cache entry(MAX)" << endl;
    cout << "elimvar:" << elimvar << endl;
    cout << "keys:";
    BOOST_FOREACH(size_t k, entryKey.GetParamSet()) {
        cout << " " << (void*)k;
    }
    cout << endl << "res:" << ret.get() << endl;
    */
    return ret;
}

// Sets each AND node of variables to marginalize to be the result of minimizing
// the respective MetaNode children of each AND node. Redundancy can be
// resolved outside.
WeightedMetaNodeList NodeManager::Minimize(MetaNodePtr root, const Scope &s,
        const DirectedGraph &embeddedpt) {
    if (root.get() == MetaNode::GetZero().get()) {
        return WeightedMetaNodeList(MetaNodeList(1, MetaNode::GetZero()), 0.0);
    }
    else if (root.get() == MetaNode::GetOne().get()) {
        return WeightedMetaNodeList(MetaNodeList(1, MetaNode::GetOne()), 1.0);
    }
    int varid = root->GetVarID();
    int card = root->GetCard();
    int elimvar = s.GetOrdering().front();

    Operation ocEntry(MIN, root, elimvar);
    OperationCache::iterator ocit = opCache.find(ocEntry);
    if ( ocit != opCache.end() ) {
        //Found result in cache
        /*
        cout << "operator: " << ocit->first.GetOperator() << endl;
        cout << "root: " << root.get() << endl;
        cout << "elimvar: " << elimvar << endl;
        cout << "Returning: " << ocit->second.get() << endl;
        */
        return ocit->second;
    }

    // Minimize each subgraph
    const vector<ANDNodePtr> &andNodes = root->GetChildren();
    vector<ANDNodePtr> newANDNodes;
    BOOST_FOREACH(ANDNodePtr i, andNodes) {
        const vector<MetaNodePtr> &metaNodes = i->GetChildren();
        vector<MetaNodePtr> newMetaNodes;
        bool terminalOnly = true;
        double weight = 1.0;
        BOOST_FOREACH(MetaNodePtr j, metaNodes) {
            WeightedMetaNodeList newMetaNodeList = Minimize(j, s, embeddedpt);
            /*
            cout << "Input lhs: " << "(w="<< w << ", rhs size=" << paramSets[i].second.size() << ")"<< endl;
            paramSets[i].first->RecursivePrint(cout); cout << endl;
            cout << "SubDD:" << "(" << varid << "," << k << ")" << endl;
            subDD->RecursivePrint(cout); cout << endl;
            */

            // Note: would have to modify for other operators
            weight *= newMetaNodeList.second;
            BOOST_FOREACH(MetaNodePtr m, newMetaNodeList.first) {
                if (!m->IsTerminal()) {
                    if (terminalOnly) {
                        newMetaNodes.clear();
                        terminalOnly = false;
                    }
                    newMetaNodes.push_back(m);
                }
                else if (newMetaNodes.empty()) {
                    newMetaNodes.push_back(m);
                }
            }
        }
        ANDNodePtr newANDNode(new MetaNode::ANDNode(i->GetWeight() * weight, newMetaNodes));
        newANDNodes.push_back(newANDNode);
    }

    // If the root is to be maximized
    if (s.VarExists(varid)) {
        // Assume node resides at bottom
        double weight = DOUBLE_MAX;
        for (unsigned int i = 0; i < newANDNodes.size(); ++i) {
            double temp = newANDNodes[i]->GetWeight();
            if (temp < weight) {
                weight = temp;
            }
        }
        newANDNodes.clear();
        vector<MetaNodePtr> newMetaNodes;
        if (weight == 0) {
            newMetaNodes.push_back(MetaNode::GetZero());
        } else {
            newMetaNodes.push_back(MetaNode::GetOne());
        }
        ANDNodePtr newAND(new MetaNode::ANDNode(weight, newMetaNodes));
        for (unsigned int i = 0; i < andNodes.size(); ++i) {
            newANDNodes.push_back(newAND);
        }
    }
    Scope var;
    var.AddVar(varid, card);
    WeightedMetaNodeList ret = CreateMetaNode(var, newANDNodes);
    opCache.insert(make_pair<Operation, WeightedMetaNodeList>(ocEntry, ret));
    opCacheMemUsage += (ocEntry.MemUsage() + sizeof(ret) + (ret.first.size() * sizeof(MetaNodePtr))) / MB_PER_BYTE;

    // Purge if op cache is too large
    if (opCacheMemUsage > OCMBLimit) {
        NodeManager::GetNodeManager()->PurgeOpCache();
    }

    if (opCacheMemUsage > maxOpCacheMemUsage) maxOpCacheMemUsage = opCacheMemUsage;
    /*
    cout << "Created cache entry(MAX)" << endl;
    cout << "elimvar:" << elimvar << endl;
    cout << "keys:";
    BOOST_FOREACH(size_t k, entryKey.GetParamSet()) {
        cout << " " << (void*)k;
    }
    cout << endl << "res:" << ret.get() << endl;
    */
    return ret;
}

WeightedMetaNodeList NodeManager::Condition(MetaNodePtr root, const Assignment &cond) {
    if (root.get() == MetaNode::GetOne().get()) {
        return WeightedMetaNodeList(MetaNodeList(1, root), 1.0);
    }
    else if (root.get() == MetaNode::GetZero().get()) {
        return WeightedMetaNodeList(MetaNodeList(1, root), 0.0);
    }

    vector<ANDNodePtr> newANDNodes;
    const vector<ANDNodePtr> &rootCh = root->GetChildren();
    int val;
    if ( (val = cond.GetVal(root->GetVarID())) != ERROR_VAL ) {
        vector<MetaNodePtr> newMetaNodes;
        bool terminalOnly = true;
        double weight = 1.0;
        BOOST_FOREACH(MetaNodePtr j, rootCh[val]->GetChildren()) {
            WeightedMetaNodeList newMetaNodeList = Condition(j, cond);

            // Note: would have to modify for other operators
            weight *= newMetaNodeList.second;
            BOOST_FOREACH(MetaNodePtr m, newMetaNodeList.first) {
                if (!m->IsTerminal()) {
                    if (terminalOnly) {
                        newMetaNodes.clear();
                        terminalOnly = false;
                    }
                    newMetaNodes.push_back(m);
                }
                else if (newMetaNodes.empty()) {
                    newMetaNodes.push_back(m);
                }
            }
        }
        ANDNodePtr newAND(new MetaNode::ANDNode(rootCh[val]->GetWeight() * weight, newMetaNodes));
        newANDNodes.push_back(newAND);
    }
    else {
        BOOST_FOREACH(ANDNodePtr i, root->GetChildren()) {
            vector<MetaNodePtr> newMetaNodes;
            double weight = 1.0;
            BOOST_FOREACH(MetaNodePtr j, i->GetChildren()) {
                WeightedMetaNodeList newMetaNodeList = Condition(j, cond);
                bool terminalOnly = true;

                // Note: would have to modify for other operators
                weight *= newMetaNodeList.second;
                BOOST_FOREACH(MetaNodePtr m, newMetaNodeList.first) {
                    if (!m->IsTerminal()) {
                        if (terminalOnly) {
                            newMetaNodes.clear();
                            terminalOnly = false;
                        }
                        newMetaNodes.push_back(m);
                    }
                    else if (newMetaNodes.empty()) {
                        newMetaNodes.push_back(m);
                    }
                }
            }
            ANDNodePtr newAND(new MetaNode::ANDNode(i->GetWeight() * weight, newMetaNodes));
            newANDNodes.push_back(newAND);
        }
    }
    WeightedMetaNodeList ret = CreateMetaNode(root->GetVarID(), root->GetCard(), newANDNodes);
    return ret;
}

/*
double NodeManager::Normalize(MetaNodePtr root) {
    if (root->IsTerminal()) {
        return 1.0;
    }
    double normValue = 0;
    BOOST_FOREACH(ANDNodePtr i, root->GetChildren()) {
        normValue += i->GetWeight();
    }
    BOOST_FOREACH(ANDNodePtr i, root->GetChildren()) {
        i->SetWeight(i->GetWeight() / normValue);
    }
    return normValue;
}
*/


void NodeManager::PrintUniqueTable(ostream &out) const {
    BOOST_FOREACH (MetaNodePtr i, ut) {
        i->Save(out); out << endl;
    }
}

void NodeManager::PrintReferenceCount(ostream &out) const {
    BOOST_FOREACH (const MetaNodePtr &i, ut) {
        out << i.get() << ":" << i.use_count() << endl;
    }
}

bool NodeManager::initialized = false;
NodeManager *NodeManager::singleton = NULL;

} // end of aomdd namespace
