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

// Be sure to test to see if sets give the same hash if items are inserted
// in different orders
size_t hash_value(const Operation &o) {
    size_t seed = 0;
    boost::hash_combine(seed, o.GetOperator());
    BOOST_FOREACH(ParamSet::value_type i, o.GetParamSet()) {
        boost::hash_combine(seed, i);
    }
    return seed;

}
bool operator==(const Operation &lhs, const Operation &rhs) {
    return lhs.GetOperator() == rhs.GetOperator() &&
            lhs.GetVarID() == rhs.GetVarID() &&
            lhs.GetParamSet() == rhs.GetParamSet();
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

vector<ApplyParamSet> NodeManager::GetParamSets(const DirectedGraph &tree,
        const vector<MetaNode *> &lhs, const vector<MetaNode *> &rhs) const {
    vector<ApplyParamSet> ret;
    // First check if rhs is terminal and lhs is size 1
    if (lhs.size() == 1 && rhs.size() == 1 && rhs[0]->IsTerminal()) {
        ret.push_back(make_pair<MetaNode *, vector<MetaNode *> >(lhs[0], rhs));
        return ret;
    }

    unordered_map<int, MetaNode *> lhsMap;
    unordered_map<int, MetaNode *> rhsMap;

    BOOST_FOREACH(MetaNode *i, lhs) {
        lhsMap[i->GetVarID()] = i;
    }
    BOOST_FOREACH(MetaNode *i, rhs) {
        rhsMap[i->GetVarID()] = i;
    }

    unordered_map<int, int> hiAncestor;
    BOOST_FOREACH(MetaNode *i, lhs) {
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

    BOOST_FOREACH(MetaNode *i, rhs) {
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
        MetaNode *paramLHS;
        vector<MetaNode *> paramRHS;
        int anc = dit->first;
        const vector<int> &dList = dit->second;
        bool fromLHS = false;
        unordered_map<int, MetaNode *>::iterator mit;
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
        ret.push_back(make_pair<MetaNode *, vector<MetaNode *> >(paramLHS, paramRHS));
    }
    return ret;

}


// Public functions below here

WeightedMetaNodeList NodeManager::CreateMetaNode(const Scope &var,
        ANDNode **ch) {
    MetaNode *newNode(new MetaNode(var, ch));
    WeightedMetaNodeList temp = SingleLevelFullReduce(newNode);
    if (temp.first.first > 1 || temp.first.second[0] != newNode) {
        return temp;
    }
    temp.second *= temp.first.second[0]->Normalize();
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
        ANDNode **ch) {
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
    ANDNode **children = new ANDNode*[card];

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
            WeightedMetaNodeList l = CreateMetaNode(restVars, valParts[i]);
            ANDNode *newNode(new ANDNode(l.second, l.first.first, l.first.second));
            children[i] = newNode;
        }
//        }
    }
    // Otherwise we are at the leaves
    else {
        for (unsigned int i = 0; i < card; i++) {
            MetaNode *terminal = ((vals[i] == 0) ? MetaNode::GetZero()
                    : MetaNode::GetOne());
            MetaNode **ANDch(new MetaNode*[1]);
            ANDch[0] = terminal;
            ANDNode *newNode(new ANDNode(vals[i], 1, ANDch));
            children[i] = newNode;
        }
    }
    return CreateMetaNode(rootVar, children);
}

WeightedMetaNodeList NodeManager::SingleLevelFullReduce(MetaNode *node) {
    // terminal check
    if (node == MetaNode::GetZero() ||
            node == MetaNode::GetOne()) {
        double weight = node == MetaNode::GetZero() ? 0.0 : 1.0;
        MetaNode **termlist(new MetaNode*[1]);
        termlist[0] = node;
        return WeightedMetaNodeList(MetaNodeList(1, termlist), weight);
    }

    unsigned int nc = node->GetCard();
    ANDNode **ch = node->GetChildren();

    bool redundant = true;
    ANDNode *temp = ch[0];
    for (unsigned int i = 1; i < nc; ++i) {
        if (temp != ch[i]) {
            redundant = false;
            break;
        }
    }

    if ( redundant ) {
        unsigned int nc = ch[0]->GetNumChildren();
        MetaNode **nodelist = ch[0]->GetChildren();
        double weight = ch[0]->GetWeight();
        delete ch[0];
        delete node;
//        cout << "Successful deletion." << endl;
        return WeightedMetaNodeList(MetaNodeList(nc, nodelist), weight);
    }
    else {
        MetaNode **nodelist(new MetaNode*[1]);
        nodelist[0] = node;
        return WeightedMetaNodeList(MetaNodeList(1, nodelist), 1.0);
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

WeightedMetaNodeList NodeManager::Apply(MetaNode *lhs,
        const vector<MetaNode *> &rhs,
        Operator op,
        const DirectedGraph &embeddedPT) {
    int varid = lhs->GetVarID();
    int card = lhs->GetCard();

    Operation ocEntry(op, lhs, rhs);
    OperationCache::iterator ocit = opCache.find(ocEntry);

    if ( ocit != opCache.end() ) {
        //Found result in cache
        /*
        cout << "lhs: " << lhs.get() << endl;
        cout << "Returning:" << ocit->second.get() << endl;
        */
        return ocit->second;
    }
    if (useTempMode) {
        ocit = opCacheTemp.find(ocEntry);
        if ( ocit != opCacheTemp.end() ) {
            return ocit->second;
        }
    }

    // Base cases
    switch(op) {
        case PROD:
            // If its a terminal the rhs must be same terminals
            if ( rhs.size() == 0 || lhs->IsTerminal() ) {
                MetaNode **nodelist(new MetaNode*[1]);
                nodelist[0] = lhs;
                return WeightedMetaNodeList(MetaNodeList(1, nodelist), 1.0);
            }
            // Look for any zeros on the rhs. Result is zero if found
            else {
                for (unsigned int i = 0; i < rhs.size(); ++i) {
                    if ( rhs[i] == MetaNode::GetZero() ) {
                        MetaNode **nodelist(new MetaNode*[1]);
                        nodelist[0] = MetaNode::GetZero();
                        return WeightedMetaNodeList(MetaNodeList(1, nodelist), 0.0);
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
                MetaNode **nodelist(new MetaNode*[1]);
                nodelist[0] = lhs;
                return WeightedMetaNodeList(MetaNodeList(1, nodelist), 1.0);
            }
            break;
        case MAX:
            if ( rhs.size() == 0 || lhs->IsTerminal() ) {
                MetaNode **nodelist(new MetaNode*[1]);
                nodelist[0] = lhs;
                return WeightedMetaNodeList(MetaNodeList(1, nodelist), 1.0);
            }
            break;
        default:
            assert(false);
            break;
    }

    // Should have detected terminals
    assert(varid >= 0);


//    vector<ANDNodePtr> children;
    ANDNode **children(new ANDNode*[card]);

    // For each value of lhs
    for (int k = 0; k < card; ++k) {
        // Get original weight
//        vector<MetaNodePtr> newChildren;
        MetaNode **newChildren;
        double weight = 1.0;
        weight *= lhs->GetChildren()[k]->GetWeight();
        vector<ApplyParamSet> paramSets;
        /*
        const vector<MetaNode *> &paramLHS =
                lhs->GetChildren()[k]->GetChildren();
                */
        MetaNode **tLHS = lhs->GetChildren()[k]->GetChildren();
        unsigned int lnc = lhs->GetChildren()[k]->GetNumChildren();
        vector<MetaNode *> paramLHS;
        for (unsigned int i = 0; i < lnc; ++i) {
            paramLHS.push_back(tLHS[i]);
        }

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
            MetaNode **tRHS = lhs->GetChildren()[k]->GetChildren();
            unsigned int rnc = lhs->GetChildren()[k]->GetNumChildren();
            vector<MetaNode *> paramRHS;
            for (unsigned int i = 0; i < rnc; ++i) {
                paramRHS.push_back(tRHS[i]);
            }
            paramSets = GetParamSets(embeddedPT, paramLHS, paramRHS);
        }
        else {
            // Not the same variable, prepare to push rhs down
            paramSets = GetParamSets(embeddedPT, paramLHS, rhs);
        }

        bool terminalOnly = true;
        bool earlyTerminate = false;

        vector<MetaNode *> tempResults;
        if (weight == 0) {
            tempResults.push_back(MetaNode::GetZero());
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
                for (unsigned int i = 0 ; i < subDD.first.first; ++i) {
                    MetaNode *m = subDD.first.second[i];
                    if (op == PROD && m == MetaNode::GetZero()) {
//                        cout << "Found a zero terminal" << endl;
                        tempResults.clear();
                        tempResults.push_back(m);
                        earlyTerminate = true;
                        break;
                    }
                    if (!m->IsTerminal()) {
                        if (terminalOnly) {
                            tempResults.clear();
                            terminalOnly = false;
                        }
                        tempResults.push_back(m);
                    }
                    else if (tempResults.empty()) {
                        tempResults.push_back(m);
                    }
                }
                delete []subDD.first.second;
                if (earlyTerminate) break;
            }
        }

        if (weight == 0) {
            assert(tempResults.size() == 1);
            assert(tempResults[0] == MetaNode::GetZero());
        }
        unsigned int nc = tempResults.size();

        newChildren = new MetaNode*[nc];
        for (unsigned int i = 0; i < nc; ++i) {
            newChildren[i] = tempResults[i];
        }
        ANDNode *newNode(new ANDNode(weight, nc, newChildren));
        children[k] = newNode;
    }
    Scope var;
    var.AddVar(varid, card);
    WeightedMetaNodeList u = CreateMetaNode(var, children);
    if (!useTempMode) {
//        opCache.insert(make_pair<Operation, WeightedMetaNodeList>(ocEntry, u));
    }
    else {
//        opCacheTemp.insert(make_pair<Operation, WeightedMetaNodeList>(ocEntry, u));
    }
    /*
    cout << "Created cache entry" << endl;
    cout << "keys:";
    BOOST_FOREACH(size_t k, entryKey.GetParamSet()) {
        cout << " " << (void*)k;
    }
    cout << endl << "res:" << u.get() << endl;
    */
    return u;
}

// Sets each AND node of variables to marginalize to be the result of summing
// the respective MetaNode children of each AND node.

// Current bug: have to check for the case where the node is not present,
// but is a child of a node that is being marginalized.
WeightedMetaNodeList NodeManager::Marginalize(MetaNode *root, const Scope &s,
        const DirectedGraph &embeddedpt, bool &sumOpPerformed) {
    if (root == MetaNode::GetZero()) {
        MetaNode **nodelist(new MetaNode*[1]);
        nodelist[0] = MetaNode::GetZero();
        return WeightedMetaNodeList(MetaNodeList(1, nodelist), 0.0);
    }
    else if (root == MetaNode::GetOne()) {
        MetaNode **nodelist(new MetaNode*[1]);
        nodelist[0] = MetaNode::GetOne();
        return WeightedMetaNodeList(MetaNodeList(1, nodelist), 1.0);
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
    if (useTempMode) {
        ocit = opCacheTemp.find(ocEntry);
        if ( ocit != opCacheTemp.end() ) {
            return ocit->second;
        }
    }

    // Marginalize each subgraph
    ANDNode **andNodes = root->GetChildren();
    ANDNode **newANDNodes(new ANDNode*[card]);
//    vector<ANDNodePtr> newANDNodes;
    for (int i = 0; i < card; ++i) {
        unsigned int nc = andNodes[i]->GetNumChildren();
        MetaNode **metaNodes = andNodes[i]->GetChildren();
        vector<MetaNode *> tempResults;
        bool terminalOnly = true;
        double weight = 1.0;
        bool checkForMargVar = false;
        bool foundMargVar = false;

        DEdge ei, ei_end;

        tie(ei, ei_end) = out_edges(varid, embeddedpt);
        while(ei != ei_end) {
            if (int(target(*ei, embeddedpt)) == elimvar) {
                checkForMargVar = true;
                break;
            }
            ei++;
        }


        for (unsigned int j = 0; j < nc; ++j) {
            if (checkForMargVar && metaNodes[j]->GetVarID() == elimvar) {
                foundMargVar = true;
            }
            WeightedMetaNodeList newMetaNodeList = Marginalize(metaNodes[j], s, embeddedpt, sumOpPerformed);
            /*
            cout << "Input lhs: " << "(w="<< w << ", rhs size=" << paramSets[i].second.size() << ")"<< endl;
            paramSets[i].first->RecursivePrint(cout); cout << endl;
            cout << "SubDD:" << "(" << varid << "," << k << ")" << endl;
            subDD->RecursivePrint(cout); cout << endl;
            */

            // Note: would have to modify for other operators
            weight *= newMetaNodeList.second;

            for (unsigned int k = 0; k < newMetaNodeList.first.first; ++k) {
                MetaNode *m = newMetaNodeList.first.second[k];
                if (!m->IsTerminal()) {
                    if (terminalOnly) {
                        tempResults.clear();
                        terminalOnly = false;
                    }
                    tempResults.push_back(m);
                }
                else if (tempResults.empty()) {
                    tempResults.push_back(m);
                }
            }
            delete []newMetaNodeList.first.second;
        }
        if (checkForMargVar && !foundMargVar) {
            weight *= s.GetVarCard(elimvar);
        }
        MetaNode **newMetaNodes(new MetaNode*[tempResults.size()]);
        for (unsigned int j = 0; j < tempResults.size(); ++j) {
            newMetaNodes[j] = tempResults[j];
        }
        ANDNode *newANDNode(new ANDNode(andNodes[i]->GetWeight() * weight, tempResults.size(),newMetaNodes));
        newANDNodes[i] = newANDNode;
    }

    // If the root is to be marginalized
    if (s.VarExists(varid)) {
//        sumOpPerformed = true;
        // Assume node resides at bottom
        double weight = 0;
        for (int i = 0; i < card; ++i) {
            weight += newANDNodes[i]->GetWeight();
        }
//        newANDNodes.clear();
//        vector<MetaNodePtr> newMetaNodes;
        MetaNode **newMetaNodes(new MetaNode*[1]);
        if (weight == 0) {
            newMetaNodes[0] = MetaNode::GetZero();
        } else {
            newMetaNodes[0] = MetaNode::GetOne();
        }
        ANDNode *newAND(new MetaNode::ANDNode(weight, 1, newMetaNodes));
        for (int i = 0; i < card; ++i) {
            newANDNodes[i] = newAND;
        }
    }
    Scope var;
    var.AddVar(varid, card);
    WeightedMetaNodeList ret = CreateMetaNode(var, newANDNodes);
    if (!useTempMode) {
//        opCache.insert(make_pair<Operation, WeightedMetaNodeList>(ocEntry, ret));
    }
    else {
//        opCacheTemp.insert(make_pair<Operation, WeightedMetaNodeList>(ocEntry, ret));
    }
    return ret;
}

// Sets each AND node of variables to marginalize to be the result of maximizing
// the respective MetaNode children of each AND node. Redundancy can be
// resolved outside.
WeightedMetaNodeList NodeManager::Maximize(MetaNode *root, const Scope &s,
        const DirectedGraph &embeddedpt) {
    if (root == MetaNode::GetZero()) {
        MetaNode **nodelist(new MetaNode*[1]);
        nodelist[0] = MetaNode::GetZero();
        return WeightedMetaNodeList(MetaNodeList(1, nodelist), 0.0);
    }
    else if (root == MetaNode::GetOne()) {
        MetaNode **nodelist(new MetaNode*[1]);
        nodelist[0] = MetaNode::GetOne();
        return WeightedMetaNodeList(MetaNodeList(1, nodelist), 1.0);
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
    if (useTempMode) {
        ocit = opCacheTemp.find(ocEntry);
        if ( ocit != opCacheTemp.end() ) {
            return ocit->second;
        }
    }

    // Maximize each subgraph
    ANDNode **andNodes = root->GetChildren();
    ANDNode **newANDNodes(new ANDNode*[card]);
//    vector<ANDNodePtr> newANDNodes;

    for (int i = 0; i < card; ++i) {
        unsigned int nc = andNodes[i]->GetNumChildren();
        MetaNode **metaNodes = andNodes[i]->GetChildren();
        vector<MetaNode*> tempResults;
        bool terminalOnly = true;
        double weight = 1.0;
        for (unsigned int j = 0; j < nc; ++j) {
            WeightedMetaNodeList newMetaNodeList = Maximize(metaNodes[j], s, embeddedpt);
            /*
            cout << "Input lhs: " << "(w="<< w << ", rhs size=" << paramSets[i].second.size() << ")"<< endl;
            paramSets[i].first->RecursivePrint(cout); cout << endl;
            cout << "SubDD:" << "(" << varid << "," << k << ")" << endl;
            subDD->RecursivePrint(cout); cout << endl;
            */

            // Note: would have to modify for other operators
            weight *= newMetaNodeList.second;
            for (unsigned int k = 0; k < newMetaNodeList.first.first; ++k) {
                MetaNode *m = newMetaNodeList.first.second[k];
                if (!m->IsTerminal()) {
                    if (terminalOnly) {
                        tempResults.clear();
                        terminalOnly = false;
                    }
                    tempResults.push_back(m);
                }
                else if (tempResults.empty()) {
                    tempResults.push_back(m);
                }
            }
        }
        MetaNode **newMetaNodes(new MetaNode*[tempResults.size()]);
        for (unsigned int j = 0; j < tempResults.size(); ++j) {
            newMetaNodes[j] = tempResults[j];
        }

        ANDNode *newANDNode(new ANDNode(andNodes[i]->GetWeight() * weight, tempResults.size(), newMetaNodes));
        newANDNodes[i] = newANDNode;
    }

    // If the root is to be maximized
    if (s.VarExists(varid)) {
        // Assume node resides at bottom
        double weight = DOUBLE_MIN;
        for (int i = 0; i < card; ++i) {
            double temp = newANDNodes[i]->GetWeight();
            if (temp > weight) {
                weight = temp;
            }
        }
//        newANDNodes.clear();
        MetaNode **newMetaNodes(new MetaNode*[1]);
        if (weight == 0) {
            newMetaNodes[0] = MetaNode::GetZero();
        } else {
            newMetaNodes[0] = MetaNode::GetOne();
        }
        ANDNode *newAND(new MetaNode::ANDNode(weight, 1, newMetaNodes));
        for (int i = 0; i < card; ++i) {
            newANDNodes[i] = newAND;
        }
    }
    Scope var;
    var.AddVar(varid, card);
    WeightedMetaNodeList ret = CreateMetaNode(var, newANDNodes);
    if (!useTempMode) {
//        opCache.insert(make_pair<Operation, WeightedMetaNodeList>(ocEntry, ret));
    }
    else {
//        opCacheTemp.insert(make_pair<Operation, WeightedMetaNodeList>(ocEntry, ret));
    }
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

WeightedMetaNodeList NodeManager::Condition(MetaNode *root, const Assignment &cond) {
    if (root == MetaNode::GetZero()) {
        MetaNode **nodelist(new MetaNode*[1]);
        nodelist[0] = MetaNode::GetZero();
        return WeightedMetaNodeList(MetaNodeList(1, nodelist), 0.0);
    }
    else if (root == MetaNode::GetOne()) {
        MetaNode **nodelist(new MetaNode*[1]);
        nodelist[0] = MetaNode::GetOne();
        return WeightedMetaNodeList(MetaNodeList(1, nodelist), 1.0);
    }

    int card = root->GetCard();

//    vector<ANDNodePtr> newANDNodes;
    ANDNode **newANDNodes(new ANDNode*[card]);
    ANDNode **rootCh = root->GetChildren();
    int val;
    if ( (val = cond.GetVal(root->GetVarID())) != ERROR_VAL ) {
        int nc = rootCh[val]->GetNumChildren();
        vector<MetaNode *> tempResults;
        bool terminalOnly = true;
        double weight = 1.0;
        for (int i = 0; i < nc; ++i) {
            WeightedMetaNodeList newMetaNodeList = Condition(rootCh[val]->GetChildren()[i], cond);

            // Note: would have to modify for other operators
            weight *= newMetaNodeList.second;
            for (unsigned int j = 0; j < newMetaNodeList.first.first; ++j) {
                MetaNode *m = newMetaNodeList.first.second[j];
                if (!m->IsTerminal()) {
                    if (terminalOnly) {
                        tempResults.clear();
                        terminalOnly = false;
                    }
                    tempResults.push_back(m);
                }
                else if (tempResults.empty()) {
                    tempResults.push_back(m);
                }
            }
        }
        MetaNode **newMetaNodes(new MetaNode*[tempResults.size()]);
        for (unsigned int i = 0; i < tempResults.size(); ++i) {
            newMetaNodes[i] = tempResults[i];
        }
        ANDNode *newAND(new ANDNode(rootCh[val]->GetWeight() * weight, tempResults.size(), newMetaNodes));
        for (int i = 0; i < card; ++i) {
            newANDNodes[i] = newAND;
        }
    }
    else {
        for (int i = 0; i < card; ++i) {
            unsigned int nc = rootCh[i]->GetNumChildren();
            vector<MetaNode *> tempResults;

            double weight = 1.0;
            for (unsigned int j = 0; j < nc; ++j) {
                WeightedMetaNodeList newMetaNodeList = Condition(rootCh[i]->GetChildren()[j], cond);
                bool terminalOnly = true;

                // Note: would have to modify for other operators
                weight *= newMetaNodeList.second;
                for (unsigned int k = 0; k < newMetaNodeList.first.first; ++k) {
                    MetaNode *m = newMetaNodeList.first.second[k];
                    if (!m->IsTerminal()) {
                        if (terminalOnly) {
                            tempResults.clear();
                            terminalOnly = false;
                        }
                        tempResults.push_back(m);
                    }
                    else if (tempResults.empty()) {
                        tempResults.push_back(m);
                    }
                }
            }
            MetaNode **newMetaNodes(new MetaNode*[tempResults.size()]);
            for (unsigned int j = 0; j < tempResults.size(); ++j) {
                newMetaNodes[j] = tempResults[j];
            }
            ANDNode *newAND(new ANDNode(rootCh[i]->GetWeight() * weight, tempResults.size(), newMetaNodes));
            newANDNodes[i] = newAND;
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
    BOOST_FOREACH (const MetaNode *i, ut) {
        i->Save(out); out << endl;
    }
}

bool NodeManager::initialized = false;
NodeManager *NodeManager::singleton = NULL;

} // end of aomdd namespace
