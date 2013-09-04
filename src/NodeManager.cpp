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

#define MAX_FUN(a, b) (a > b ? a : b)

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

void NodeManager::GetParamSets2(
        const vector<MetaNodePtr> &lhs, const vector<MetaNodePtr> &rhs, vector<ApplyParamSet> &ret) const {

    unordered_map<int, int> lhsMap;
    unordered_map<int, int> rhsMap;
    set<int> lhsVarsSet;
    set<int> rhsVarsSet;

    // Make an index to the meta nodes by their variable
    for (unsigned int i = 0; i < lhs.size(); ++i) {
        lhsMap[lhs[i]->GetVarID()] = i;
        lhsVarsSet.insert(lhs[i]->GetVarID());
    }
    for (unsigned int i = 0; i < rhs.size(); ++i) {
        rhsMap[rhs[i]->GetVarID()] = i;
        rhsVarsSet.insert(rhs[i]->GetVarID());
    }

    /*
    cout << "begin search" << endl;
    for (size_t i = 0; i < descendants->size(); ++i) {
        cout << i << " : ";
        BOOST_FOREACH(int s, (*descendants)[i]) {
            cout << " " << s;
        }
        cout << endl;
    }
    */

    BOOST_FOREACH(int i, (*ordering)) {
        if (lhsVarsSet.empty() || rhsVarsSet.empty()) break;

        if (lhsVarsSet.find(i) != lhsVarsSet.end()) {
	        MetaNodePtr paramLHS;
	        vector<MetaNodePtr> paramRHS;
	        // if it exists in both, we need to find which gives a larger set
	        if (rhsVarsSet.find(i) != rhsVarsSet.end()) {
	            set<int> tempLHS;
	            BOOST_FOREACH(int s, rhsVarsSet) {
	                if ((*descendants)[i].find(s) != (*descendants)[i].end()) {
	                    tempLHS.insert(s);
	                }
	            }
	            set<int> tempRHS;
	            BOOST_FOREACH(int s, lhsVarsSet) {
	                if ((*descendants)[i].find(s) != (*descendants)[i].end()) {
	                    tempRHS.insert(s);
	                }
	            }

	            if (tempLHS.size() >= tempRHS.size()) {
		            paramLHS = lhs[lhsMap[i]];
		            lhsVarsSet.erase(i);
		            BOOST_FOREACH(int s, tempLHS) {
		                paramRHS.push_back(rhs[rhsMap[s]]);
		                rhsVarsSet.erase(s);
		            }
	            }
	            else {
		            paramLHS = rhs[rhsMap[i]];
		            rhsVarsSet.erase(i);
		            BOOST_FOREACH(int s, tempRHS) {
		                paramRHS.push_back(lhs[lhsMap[s]]);
		                lhsVarsSet.erase(s);
		            }
	            }
	        }
	        else {
	            paramLHS = lhs[lhsMap[i]];
	            lhsVarsSet.erase(i);

	            BOOST_FOREACH(int s, rhsVarsSet) {
	                if ((*descendants)[i].find(s) != (*descendants)[i].end()) {
	                    paramRHS.push_back(rhs[rhsMap[s]]);
	                }
	            }

	            BOOST_FOREACH(MetaNodePtr m, paramRHS) {
	                rhsVarsSet.erase(m->GetVarID());
	            }
	        }
            ret.push_back(make_pair(paramLHS, paramRHS));
        }
        // does the current variable exist only in rhs?
        else if (rhsVarsSet.find(i) != rhsVarsSet.end()) {
	        MetaNodePtr paramLHS;
	        vector<MetaNodePtr> paramRHS;
            paramLHS = rhs[rhsMap[i]];
            rhsVarsSet.erase(i);

            BOOST_FOREACH(int s, lhsVarsSet) {
                if ((*descendants)[i].find(s) != (*descendants)[i].end()) {
                    paramRHS.push_back(lhs[lhsMap[s]]);
                }
            }

            BOOST_FOREACH(MetaNodePtr m, paramRHS) {
                lhsVarsSet.erase(m->GetVarID());
            }
            ret.push_back(make_pair(paramLHS, paramRHS));
        }
    }

    BOOST_FOREACH(int s, lhsVarsSet) {
        if (s < 0 && rhsVarsSet.find(s) != rhsVarsSet.end()) {
            ret.push_back(make_pair<MetaNodePtr, vector<MetaNodePtr> >(lhs[lhsMap[s]], vector<MetaNodePtr>(1, rhs[rhsMap[s]])));
            rhsVarsSet.erase(s);
        }
        else {
            ret.push_back(make_pair<MetaNodePtr, vector<MetaNodePtr> >(lhs[lhsMap[s]], vector<MetaNodePtr>()));
        }
    }

    BOOST_FOREACH(int s, rhsVarsSet) {
        ret.push_back(make_pair<MetaNodePtr, vector<MetaNodePtr> >(rhs[rhsMap[s]], vector<MetaNodePtr>()));
    }
}
void NodeManager::GetParamSets3(
        const vector<MetaNodePtr> &lhs, const vector<MetaNodePtr> &rhs, vector<ApplyParamSet> &ret) const {

    unordered_map<int, int> lhsMap;
    unordered_map<int, int> rhsMap;
    set<int> lhsVarsSet;
    set<int> rhsVarsSet;

    // Make an index to the meta nodes by their variable
    for (unsigned int i = 0; i < lhs.size(); ++i) {
        lhsMap[lhs[i]->GetVarID()] = i;
        lhsVarsSet.insert(lhs[i]->GetVarID());
    }
    for (unsigned int i = 0; i < rhs.size(); ++i) {
        rhsMap[rhs[i]->GetVarID()] = i;
        rhsVarsSet.insert(rhs[i]->GetVarID());
    }

    /*
    cout << "begin search" << endl;
    for (size_t i = 0; i < descendants->size(); ++i) {
        cout << i << " : ";
        BOOST_FOREACH(int s, (*descendants)[i]) {
            cout << " " << s;
        }
        cout << endl;
    }
    */

    while(!lhsVarsSet.empty() && !rhsVarsSet.empty()) {
        int largestSetSize = -1;
        int side; // 0 left, 1 right
        int root = -1;
        set<int> inters;
        BOOST_FOREACH(int l, lhsVarsSet) {
            set<int> temp;
            if (l < 0) {
                BOOST_FOREACH(int s, rhsVarsSet) {
                    if (s < 0) {
                        temp.insert(s);
                    }
                }
            }
            else {
//	            cout << "testing (from lhs) " << l << endl;
                BOOST_FOREACH(int s, rhsVarsSet) {
                    if ((*descendants)[l].find(s) != (*descendants)[l].end()) {
                        temp.insert(s);
                    }
                }
            }
            int size = int(temp.size());
            if (size > largestSetSize) {
                largestSetSize = size;
                root = l;
                inters = temp;
                side = 0;
            }
        }
        BOOST_FOREACH(int l, rhsVarsSet) {
            set<int> temp;
            if (l < 0) {
                BOOST_FOREACH(int s, lhsVarsSet) {
                    if (s < 0) {
                        temp.insert(s);
                    }
                }
            }
            else {
//                cout << "testing (from rhs)" << l << endl;
                BOOST_FOREACH(int s, lhsVarsSet) {
                    if ((*descendants)[l].find(s) != (*descendants)[l].end()) {
                        temp.insert(s);
                    }
                }
            }
            int size = int(temp.size());
            if (size > largestSetSize) {
                largestSetSize = size;
                root = l;
                inters = temp;
                side = 1;
            }
        }


        MetaNodePtr paramLHS;
        vector<MetaNodePtr> paramRHS;
        if (side == 0) {
            paramLHS = lhs[lhsMap[root]];
            lhsVarsSet.erase(root);
            BOOST_FOREACH(int s, inters) {
                paramRHS.push_back(rhs[rhsMap[s]]);
                rhsVarsSet.erase(s);
            }
        }
        else {
            paramLHS = rhs[rhsMap[root]];
            rhsVarsSet.erase(root);
            BOOST_FOREACH(int s, inters) {
                paramRHS.push_back(lhs[lhsMap[s]]);
                lhsVarsSet.erase(s);
            }

        }
        ret.push_back(make_pair<MetaNodePtr, vector<MetaNodePtr> >(paramLHS, paramRHS));
    }

    BOOST_FOREACH(int s, lhsVarsSet) {
        if (s < 0 && rhsVarsSet.find(s) != rhsVarsSet.end()) {
            ret.push_back(make_pair<MetaNodePtr, vector<MetaNodePtr> >(lhs[lhsMap[s]], vector<MetaNodePtr>(1, rhs[rhsMap[s]])));
            rhsVarsSet.erase(s);
        }
        else {
            ret.push_back(make_pair<MetaNodePtr, vector<MetaNodePtr> >(lhs[lhsMap[s]], vector<MetaNodePtr>()));
        }
    }

    BOOST_FOREACH(int s, rhsVarsSet) {
        ret.push_back(make_pair<MetaNodePtr, vector<MetaNodePtr> >(rhs[rhsMap[s]], vector<MetaNodePtr>()));
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

    unordered_map<int, int> lhsMap;
    unordered_map<int, int> rhsMap;

    /*
    google::dense_hash_map<int, MetaNodePtr> lhsMap;
    lhsMap.set_empty_key(EMPTY_KEY);
    lhsMap.set_deleted_key(DELETED_KEY);
    google::dense_hash_map<int, MetaNodePtr> rhsMap;
    rhsMap.set_empty_key(EMPTY_KEY);
    rhsMap.set_deleted_key(DELETED_KEY);
    */

    // Make an index to the meta nodes by their variable
    for (unsigned int i = 0; i < lhs.size(); ++i) {
        lhsMap[lhs[i]->GetVarID()] = i;
    }
    for (unsigned int i = 0; i < rhs.size(); ++i) {
        rhsMap[rhs[i]->GetVarID()] = i;
    }

    // For each variable on the lhs, find the highest ancestor
    // which exists on the rhs.
    unordered_map<int, int> hiAncestor;

    /*
    google::dense_hash_map<int, int> hiAncestor;
    hiAncestor.set_empty_key(EMPTY_KEY);
    */

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

    // For each variable on the rhs, find the highest ancestor
    // which exists on the lhs.
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

    // make a list of descendants for each variable, based on the
    // ancestor mapping above
    unordered_map<int, vector<int> > descendants;
    /*
    google::dense_hash_map<int, vector<int> > descendants;
    descendants.set_empty_key(EMPTY_KEY);
    */

    unordered_map<int, int>::iterator it = hiAncestor.begin();
//    google::dense_hash_map<int, int>::iterator it = hiAncestor.begin();

    for (; it != hiAncestor.end(); ++it) {
        descendants[it->second].push_back(it->first);
    }

    unordered_map<int, vector<int> >::iterator dit = descendants.begin();
//    google::dense_hash_map<int, vector<int> >::iterator dit = descendants.begin();
    for (; dit != descendants.end(); ++dit) {
        MetaNodePtr paramLHS;
        vector<MetaNodePtr> paramRHS;
        int anc = dit->first;
        const vector<int> &dList = dit->second;
        bool fromLHS = false;
        unordered_map<int, int>::iterator mit;
//        google::dense_hash_map<int, MetaNodePtr>::iterator mit;
        if ( (mit = lhsMap.find(anc)) != lhsMap.end() ) {
            paramLHS = lhs[mit->second];
            fromLHS = true;
            lhsMap.erase(mit);
        }
        else if ( (mit = rhsMap.find(anc)) != rhsMap.end() ) {
            paramLHS = rhs[mit->second];
            rhsMap.erase(mit);
        }
        else {
            // Problem if it gets here
            assert(false);
        }
        BOOST_FOREACH(int i, dList) {
            if ( fromLHS && (mit = rhsMap.find(i)) != rhsMap.end() ) {
                paramRHS.push_back(rhs[mit->second]);
                rhsMap.erase(mit);
            }
            else if ( (mit = lhsMap.find(i)) != lhsMap.end() ) {
                paramRHS.push_back(lhs[mit->second]);
                lhsMap.erase(mit);
            }
        }
        ret.push_back(make_pair<MetaNodePtr, vector<MetaNodePtr> >(paramLHS, paramRHS));
    }
    return ret;

}


// Public functions below here

ANDNodePtr NodeManager::CreateMetaNode(int varid, unsigned int card,
        const vector<ANDNodePtr> &ch) {
    MetaNodePtr newNode(new MetaNode(varid, card, ch));
//    MetaNodePtr newNode = make_shared<MetaNode>(varid, card, ch);
//    cout << newNode->refs << endl;
    ANDNodePtr temp = SingleLevelFullReduce(newNode);
//    cout << newNode->refs << endl;
    if (temp->GetChildren().size() > 1 || temp->GetChildren()[0].get() != newNode.get()) {
        return temp;
    }
    if (!cmOnly)
        temp->ScaleWeight(temp->GetChildren()[0]->Normalize());
    temp = LookupUT(temp);
//    cout << newNode->refs << endl;
    return temp;

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

/*
WeightedMetaNodeList NodeManager::CreateMetaNode(int varid, unsigned int card,
        const vector<ANDNodePtr> &ch) {
    Scope var;
    var.AddVar(varid, card);
    return CreateMetaNode(var, ch);
}
*/

ANDNodePtr NodeManager::CreateMetaNode(const Scope &vars,
        const vector<double> &vals) {
    assert(vars.GetCard() == vals.size());
    int rootVarID = vars.GetOrdering().front();
    unsigned int card = vars.GetVarCard(rootVarID);
    vector<ANDNodePtr> children;

    // Need to split the values vector if we are not at a leaf
    if (card != vals.size()) {
        Scope restVars(vars);
        restVars.RemoveVar(rootVarID);

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
//            ANDNodePtr newNode(new MetaNode::ANDNode(l.second, l.first));
            children.push_back(CreateMetaNode(restVars, valParts[i]));
        }
//        }
    }
    // Otherwise we are at the leaves
    else {
        for (unsigned int i = 0; i < card; i++) {
            const MetaNodePtr &terminal = ((vals[i] == 0) ? MetaNode::GetZero()
                    : MetaNode::GetOne());
            children.push_back(ANDNodePtr(new ANDNode(vals[i], MetaNodeList(1, terminal))));
        }
    }
    return CreateMetaNode(rootVarID, card, children);
}

ANDNodePtr NodeManager::SingleLevelFullReduce(MetaNodePtr node) {
    // terminal check
    if (node.get() == MetaNode::GetZero().get() ||
            node.get() == MetaNode::GetOne().get()) {
        double weight = node.get() == MetaNode::GetZero().get() ? 0.0 : 1.0;
        return ANDNodePtr(new ANDNode(weight, MetaNodeList(1,node)));
//        return make_shared<ANDNode>(weight, MetaNodeList(1,node));
//        return ANDNodePtr(new MetaNode::ANDNode(weight, MetaNodeList(1,node)));
//        return WeightedMetaNodeList(MetaNodeList(1, node), weight);
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
//        return make_shared<ANDNode>(ch[0]->GetWeight(), ch[0]->GetChildren());
        return ANDNodePtr(new ANDNode(ch[0]->GetWeight(), ch[0]->GetChildren()));
    }
    else {
//        return make_shared<ANDNode>(1.0, MetaNodeList(1, node));
        return ANDNodePtr(new ANDNode(1.0, MetaNodeList(1, node)));
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

ANDNodePtr NodeManager::Apply(MetaNodePtr lhs,
        const MetaNodeList &rhs,
        Operator op,
        const DirectedGraph &embeddedPT) {
    int varid = lhs->GetVarID();
    int card = lhs->GetCard();

#ifdef USE_OPCACHE
    Operation ocEntry(op, lhs, rhs);
    OperationCache::iterator ocit = opCache.find(ocEntry);

    if ( ocit != opCache.end() ) {
        return ocit->second;
    }
#endif

    // Base cases
    switch(op) {
        case PROD:
            // If its a terminal the rhs must be same terminals
            if ( rhs.size() == 0 || lhs->IsTerminal() ) {
//                return make_shared<ANDNode>(1.0, MetaNodeList(1, lhs));
                return ANDNodePtr(new ANDNode(1.0, MetaNodeList(1, lhs)));
            }
            // Look for any zeros on the rhs. Result is zero if found
            else {
                for (unsigned int i = 0; i < rhs.size(); ++i) {
                    if ( rhs[i].get() == MetaNode::GetZero().get() ) {
//                        return make_shared<ANDNode>(0.0, MetaNodeList(1, MetaNode::GetZero()));
                        return ANDNodePtr(new ANDNode(0.0, MetaNodeList(1, MetaNode::GetZero())));
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
//                  return make_shared<ANDNode>(1.0, MetaNodeList(1, lhs));
                return ANDNodePtr(new ANDNode(1.0, MetaNodeList(1, lhs)));
//                return WeightedMetaNodeList(MetaNodeList(1, lhs), 1.0);
            }
            break;
        case MAX:
            if ( rhs.size() == 0 || lhs->IsTerminal() ) {
//                return make_shared<ANDNode>(1.0, MetaNodeList(1, lhs));
                return ANDNodePtr(new ANDNode(1.0, MetaNodeList(1, lhs)));
//                return WeightedMetaNodeList(MetaNodeList(1, lhs), 1.0);
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
                    weight = MAX_FUN(weight, rhsWeight);
                    break;
                default:
                    assert(false);
                    break;
            }
//            paramSets = GetParamSets(embeddedPT, paramLHS, rhs[0]->GetChildren()[k]->GetChildren());
            GetParamSets3(paramLHS, rhs[0]->GetChildren()[k]->GetChildren(), paramSets);
        }
        else {
            // Not the same variable, prepare to push rhs down
//            paramSets = GetParamSets(embeddedPT, paramLHS, rhs);
            GetParamSets3(paramLHS, rhs, paramSets);
        }

        bool terminalOnly = true;
        bool earlyTerminate = false;
        if (weight == 0) {
            newChildren.push_back(MetaNode::GetZero());
        }

        else {
            // For each parameter set
            /*
            cout << "New param set" << endl;
            BOOST_FOREACH(ApplyParamSet aps, paramSets) {
	            cout << "lhs: " << aps.first->GetVarID() << ", ";
	            cout << "rhs:";
	            BOOST_FOREACH(MetaNodePtr r, aps.second) {
	                cout << " " << r->GetVarID();
	            }
	            cout << endl;
            }
            */
            BOOST_FOREACH(ApplyParamSet aps, paramSets) {
                ANDNodePtr subDD = Apply(aps.first, aps.second, op, embeddedPT);

            /*

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
                weight *= subDD->GetWeight();
                BOOST_FOREACH(MetaNodePtr m, subDD->GetChildren()) {
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
        children.push_back(ANDNodePtr(new ANDNode(weight, newChildren)));
    }
    /*
    Scope var;
    var.AddVar(varid, card);
    */
    ANDNodePtr u = CreateMetaNode(varid, card, children);

#ifdef USE_OPCACHE
    opCache.insert(make_pair<Operation, ANDNodePtr>(ocEntry, u));
    opCacheMemUsage += (ocEntry.MemUsage() + sizeof(u) + (u->GetChildren().size() * sizeof(MetaNodePtr))) / MB_PER_BYTE;

    // Purge if op cache is too large
    if (opCacheMemUsage > OCMBLimit) {
        NodeManager::GetNodeManager()->PurgeOpCache();
    }

    if (opCacheMemUsage > maxOpCacheMemUsage) maxOpCacheMemUsage = opCacheMemUsage;
#endif
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
ANDNodePtr NodeManager::Marginalize(MetaNodePtr root, const Scope &s,
        const DirectedGraph &elimChain) {
    if (root.get() == MetaNode::GetZero().get()) {
//        return make_shared<ANDNode>(0.0, MetaNodeList(1, MetaNode::GetZero()));
        return ANDNodePtr(new MetaNode::ANDNode(0.0, MetaNodeList(1, MetaNode::GetZero())));
    }
    else if (root.get() == MetaNode::GetOne().get()) {
//        return make_shared<ANDNode>(1.0, MetaNodeList(1, MetaNode::GetOne()));
        return ANDNodePtr(new MetaNode::ANDNode(1.0, MetaNodeList(1, MetaNode::GetOne())));
    }
    int varid = root->GetVarID();
    int card = root->GetCard();
    int elimvar = s.GetOrdering().front();

#ifdef USE_OPCACHE
    Operation ocEntry(MARGINALIZE, root, elimvar);
    OperationCache::iterator ocit = opCache.find(ocEntry);
    if ( ocit != opCache.end() ) {
        //Found result in cache
        return ocit->second;
    }
#endif

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
            ANDNodePtr newMetaNodeList = Marginalize(j, s, elimChain);
            /*
            cout << "Input lhs: " << "(w="<< w << ", rhs size=" << paramSets[i].second.size() << ")"<< endl;
            paramSets[i].first->RecursivePrint(cout); cout << endl;
            cout << "SubDD:" << "(" << varid << "," << k << ")" << endl;
            subDD->RecursivePrint(cout); cout << endl;
            */

            // Note: would have to modify for other operators
            weight *= newMetaNodeList->GetWeight();

            BOOST_FOREACH(MetaNodePtr m, newMetaNodeList->GetChildren()) {
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
//        ANDNodePtr newANDNode = make_shared<ANDNode>(i->GetWeight() * weight, newMetaNodes);
        newANDNodes.push_back(ANDNodePtr(new ANDNode(i->GetWeight() * weight, newMetaNodes)));
    }

    // If the root is to be marginalized
    if (s.VarExists(varid)) {
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
//        ANDNodePtr newAND = make_shared<ANDNode>(weight, newMetaNodes);
//        for (unsigned int i = 0; i < andNodes.size(); ++i) {
            newANDNodes.push_back(ANDNodePtr(new ANDNode(weight, newMetaNodes)));
//        }
    }
    /*
    Scope var;
    var.AddVar(varid, card);
    */
    ANDNodePtr ret = CreateMetaNode(varid, card, newANDNodes);
#ifdef USE_OPCACHE
    opCache.insert(make_pair<Operation, ANDNodePtr>(ocEntry, ret));
    opCacheMemUsage += (ocEntry.MemUsage() + sizeof(ret) + (ret->GetChildren().size() * sizeof(MetaNodePtr))) / MB_PER_BYTE;

    // Purge if op cache is too large
    if (opCacheMemUsage > OCMBLimit) {
        NodeManager::GetNodeManager()->PurgeOpCache();
    }

    if (opCacheMemUsage > maxOpCacheMemUsage) maxOpCacheMemUsage = opCacheMemUsage;
#endif
    return ret;
}

double NodeManager::MarginalizeFast(MetaNodePtr root, const Scope &s,
        const set<int> &relevantVars) {
    if (root.get() == MetaNode::GetZero().get()) {
        return 0.0;
    }
    else if (root.get() == MetaNode::GetOne().get()) {
        return 1.0;
    }
    /*
    int varid = root->GetVarID();
    int card = root->GetCard();
    */
    int elimvar = s.GetOrdering().front();

    /*
    // Expand only nodes in the chain
    // If a leaf in the *diagram* is reached and it is not the elimination
    // variable, multiply the AND weight by the elimination variable's cardinality
    set<int> relevantVars;
    tie(ei, ei_end) = out_edges(varid, elimChain);
    while (ei != ei_end) {
        int child = target(*ei, elimChain);
        relevantVars.insert(child);
        tie(ei, ei_end) = out_edges(child, elimChain);
    }
    */

    // idea: do a bfs visit and store the nodes encountered
    queue<MetaNode*> q;
    vector<MetaNode*> visitOrder;
    set<MetaNode*> visited;
    set<ANDNode*> visitedAND;

    q.push(root.get());
    while (!q.empty()) {
        MetaNode *c = q.front();
        visitOrder.push_back(c);
        q.pop();
        BOOST_FOREACH(ANDNodePtr a, c->GetChildren()) {
            visitedAND.insert(a.get());
            BOOST_FOREACH(MetaNodePtr m, a->GetChildren()) {
                if (visited.find(m.get()) == visited.end() &&
                        relevantVars.find(m->GetVarID()) != relevantVars.end()) {
	                q.push(m.get());
                }
                visited.insert(m.get());
            }
        }
    }

    set<ANDNode*> receivedWeight;

    for (int i = visitOrder.size() - 1; i >= 0; --i) {
        MetaNode *&m = visitOrder[i];
        if (s.VarExists(m->GetVarID())) {
            double w = 0.0;
            BOOST_FOREACH(ANDNodePtr a, m->GetChildren()) {
                w += a->GetWeight();
            }
            /*
            if (i == 0) {
                assert(m == root.get());
                return w;
            }
            */
            // propagate weight directly to parents -- reassign children
            BOOST_FOREACH(ANDNode *i, m->GetParents()) {
                i->ScaleWeight(w);
                vector<MetaNodePtr> &aCh = i->GetChildren();
                for (size_t j = 0; j < aCh.size(); ++j) {
                    if (m == aCh[j].get()) {
                        if (w == 0) {
                            aCh.clear();
                            aCh.push_back(MetaNode::GetZero());
                        }
                        else {
                            aCh.erase(aCh.begin() + j);
                        }
                        break;
                    }
                }
                if (aCh.empty()) {
                    aCh.push_back(MetaNode::GetOne());
                }
                receivedWeight.insert(i);
            }
        }
        else {
            // First check if the ANDNodes connect to relevant variables
            BOOST_FOREACH(ANDNodePtr a, m->GetChildren()) {
                if (receivedWeight.find(a.get()) != receivedWeight.end()) continue;
                bool hasRelevantChild = false;
                BOOST_FOREACH(MetaNodePtr mm, a->GetChildren()) {
                    if (relevantVars.find(mm->GetVarID()) !=
                            relevantVars.end()) {
                        hasRelevantChild = true;
                        break;
                    }
                }
                // multiply by elimination var scope size if not
                if (!hasRelevantChild) {
                    a->ScaleWeight(s.GetVarCard(elimvar));
                }
            }

            if (m->IsRedundant()) {
                double w = m->GetChildren()[0]->GetWeight();
                BOOST_FOREACH(ANDNode *a, m->GetParents()) {
                    a->ScaleWeight(w);
                    receivedWeight.insert(a);
                    vector<MetaNodePtr> &aCh = a->GetChildren();
                    MetaNodePtr mCh = m->GetChildren()[0]->GetChildren()[0];
                    if (mCh == MetaNode::GetZero()) {
                        for (size_t j = 0; j < aCh.size(); ++j)  {
                            aCh[j]->RemoveParent(a);
                        }
                        aCh.clear();
                        aCh.push_back(MetaNode::GetZero());
                    }
                    else {
                        for (size_t j = 0; j < aCh.size(); ++j)  {
                            if (m == aCh[j].get()) {
	                            aCh[j]->RemoveParent(a);
                                aCh.erase(aCh.begin() +j);
                                break;
                            }
                        }
                    }
                    if (aCh.empty()) {
                        aCh.push_back(MetaNode::GetOne());
                    }
                }
            }
            else {
            // Normalize the node and promote its weights to the parents
	            double w = m->Normalize();
	            BOOST_FOREACH(ANDNode *a, m->GetParents()) {
	                if (m != root.get() && visitedAND.find(a) == visitedAND.end()) continue;
	                a->ScaleWeight(w);
	            }
            }
        }
    }
    return 1.0;
}

// Sets each AND node of variables to marginalize to be the result of maximizing
// the respective MetaNode children of each ANDNodePtr can be
// resolved outside.
ANDNodePtr NodeManager::Maximize(MetaNodePtr root, const Scope &s,
        const DirectedGraph &embeddedpt) {
    if (root.get() == MetaNode::GetZero().get()) {
//        return make_shared<ANDNode>(0.0, MetaNodeList(1, MetaNode::GetZero()));
        return ANDNodePtr(new ANDNode(0.0, MetaNodeList(1, MetaNode::GetZero())));
    }
    else if (root.get() == MetaNode::GetOne().get()) {
//        return make_shared<ANDNode>(1.0, MetaNodeList(1, MetaNode::GetOne()));
        return ANDNodePtr(new ANDNode(1.0, MetaNodeList(1, MetaNode::GetOne())));
    }
    int varid = root->GetVarID();
    int card = root->GetCard();

#ifdef USE_OPCACHE
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
#endif

    // Maximize each subgraph
    const vector<ANDNodePtr> &andNodes = root->GetChildren();
    vector<ANDNodePtr> newANDNodes;
    BOOST_FOREACH(ANDNodePtr i, andNodes) {
        const vector<MetaNodePtr> &metaNodes = i->GetChildren();
        vector<MetaNodePtr> newMetaNodes;
        bool terminalOnly = true;
        double weight = 1.0;
        BOOST_FOREACH(MetaNodePtr j, metaNodes) {
            ANDNodePtr newMetaNodeList = Maximize(j, s, embeddedpt);
            /*
            cout << "Input lhs: " << "(w="<< w << ", rhs size=" << paramSets[i].second.size() << ")"<< endl;
            paramSets[i].first->RecursivePrint(cout); cout << endl;
            cout << "SubDD:" << "(" << varid << "," << k << ")" << endl;
            subDD->RecursivePrint(cout); cout << endl;
            */

            // Note: would have to modify for other operators
            weight *= newMetaNodeList->GetWeight();
            BOOST_FOREACH(MetaNodePtr m, newMetaNodeList->GetChildren()) {
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
//        ANDNodePtr newANDNode = make_shared<ANDNode>(i->GetWeight() * weight, newMetaNodes);
        newANDNodes.push_back(ANDNodePtr(new ANDNode(i->GetWeight() * weight, newMetaNodes)));
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
        ANDNodePtr newAND(new ANDNode(weight, newMetaNodes));
        for (unsigned int i = 0; i < andNodes.size(); ++i) {
            newANDNodes.push_back(newAND);
        }
    }
    /*
    Scope var;
    var.AddVar(varid, card);
    */
    ANDNodePtr ret = CreateMetaNode(varid, card, newANDNodes);

#ifdef USE_OPCACHE
    opCache.insert(make_pair<Operation, ANDNodePtr>(ocEntry, ret));
    opCacheMemUsage += (ocEntry.MemUsage() + sizeof(ret) + (ret->GetChildren().size() * sizeof(MetaNodePtr))) / MB_PER_BYTE;

    // Purge if op cache is too large
    if (opCacheMemUsage > OCMBLimit) {
        NodeManager::GetNodeManager()->PurgeOpCache();
    }

    if (opCacheMemUsage > maxOpCacheMemUsage) maxOpCacheMemUsage = opCacheMemUsage;
#endif
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

// Assumes variable is eliminated only once during the entire run
/*
double NodeManager::MaximizeFast(MetaNodePtr root, const Scope &s,
        const DirectedGraph &embeddedpt) {
    if (root.get() == MetaNode::GetZero().get()) {
        return 0.0;
    }
    else if (root.get() == MetaNode::GetOne().get()) {
        return 1.0;
    }
//    int varid = root->GetVarID();
//    int card = root->GetCard();

    // Maximize each subgraph
    vector<ANDNodePtr> &andNodes = root->GetChildren();
    BOOST_FOREACH(ANDNodePtr i, andNodes) {
        vector<MetaNodePtr> &metaNodes = i->GetChildren();
//        vector<MetaNodePtr> newMetaNodes;
//        bool terminalOnly = true;
        double weight = 1.0;
        for (size_t j = 0; j < metaNodes.size(); ++j) {
            if (s.VarExists(metaNodes[j]->GetVarID())) {
                double w = DOUBLE_MIN;
                vector<ANDNodePtr> &andNodesJ = metaNodes[j]->GetChildren();
                for (unsigned int k = 0; k < andNodesJ.size(); ++k) {
                    double temp = andNodesJ[k]->GetWeight();
                    if (temp > w) {
                        w = temp;
                    }
                }
		        // propagate weight directly to parents
                set<ANDNodePtr> &parents = metaNodes[j]->GetParents();
                set<ANDNodePtr>::iterator it = parents.begin();
                for (; it != parents.end(); ++it) {
                    (*it)->SetWeight((*it)->GetWeight() * w);
                }
		        if (w == 0) {
		            metaNodes[j] = MetaNode::GetZero();
		        } else {
		            metaNodes[j] = MetaNode::GetOne();
		        }
		        cout << w << endl;
		        weight *= w;
            }
            else {
                weight *= MaximizeFast(metaNodes[j], s, embeddedpt);
            }
            cout << "Input lhs: " << "(w="<< w << ", rhs size=" << paramSets[i].second.size() << ")"<< endl;
            paramSets[i].first->RecursivePrint(cout); cout << endl;
            cout << "SubDD:" << "(" << varid << "," << k << ")" << endl;
            subDD->RecursivePrint(cout); cout << endl;


            // Note: would have to modify for other operators
        }

        // Will need to consider if the weights on the AND nodes become equal
        // Do a check to see if the metaNodes[j] is redundant
        size_t orig_size = metaNodes.size();
        vector<int> toRemove;
        for (size_t j = 0; j < orig_size; ++j) {
            if (metaNodes[j]->IsRedundant()) {
                toRemove.push_back(j);
                BOOST_FOREACH(MetaNodePtr k, metaNodes[j]->GetChildren()[0]->GetChildren()) {
                    metaNodes.push_back(k);;
                }
            }
        }
        BOOST_FOREACH(int j, toRemove) {
            metaNodes.erase(metaNodes.begin() + j);
        }
        i->SetWeight(i->GetWeight()*weight);
    }
    cout << root->GetVarID() << endl;
    cout << "Normalizing" << endl;
    double norm = root->Normalize();
    cout << "val=" << norm << endl;
    return norm;
}
*/

// Non-recursive way
double NodeManager::MaximizeFast(MetaNodePtr root, const Scope &s,
        const set<int> &relevantVars) {
    if (root.get() == MetaNode::GetZero().get()) {
        return 0.0;
    }
    else if (root.get() == MetaNode::GetOne().get()) {
        return 1.0;
    }

    // idea: do a dfs visit and store the nodes encountered
    queue<MetaNode*> q;
    vector<MetaNode*> visitOrder;
    set<MetaNode*> visited;

    q.push(root.get());
    while (!q.empty()) {
        MetaNode *c = q.front();
        visitOrder.push_back(c);
        q.pop();
        BOOST_FOREACH(ANDNodePtr a, c->GetChildren()) {
            BOOST_FOREACH(MetaNodePtr m, a->GetChildren()) {
                if (visited.find(m.get()) == visited.end() &&
                        relevantVars.find(m->GetVarID()) != relevantVars.end()) {
	                visited.insert(m.get());
	                q.push(m.get());
                }
            }
        }
    }

    for (int i = visitOrder.size() - 1; i >= 0; --i) {
        MetaNode *&m = visitOrder[i];
        if (s.VarExists(m->GetVarID())) {
            double w = DOUBLE_MIN;
            BOOST_FOREACH(ANDNodePtr a, m->GetChildren()) {
                double temp = a->GetWeight();
                if (temp > w) {
                    w = temp;
                }
            }
            /*
            if (i == 0) {
                assert(m == root.get());
                return w;
            }
            */
            // propagate weight directly to parents -- reassign children
            BOOST_FOREACH(ANDNode *i, m->GetParents()) {
                i->ScaleWeight(w);
                vector<MetaNodePtr> &aCh = i->GetChildren();
                for (size_t j = 0; j < aCh.size(); ++j) {
                    if (m == aCh[j].get()) {
                        if (w == 0) {
                            aCh.clear();
                            aCh.push_back(MetaNode::GetZero());
                        }
                        else {
                            aCh.erase(aCh.begin() + j);
                        }
                        break;
                    }
                }
                if (aCh.empty()) {
                    aCh.push_back(MetaNode::GetOne());
                }
            }
        }
        else {
            if (m->IsRedundant()) {
                double w = m->GetChildren()[0]->GetWeight();
                BOOST_FOREACH(ANDNode *a, m->GetParents()) {
                    a->ScaleWeight(w);
                    vector<MetaNodePtr> &aCh = a->GetChildren();
                    MetaNodePtr mCh = m->GetChildren()[0]->GetChildren()[0];
                    if (mCh == MetaNode::GetZero()) {
                        for (size_t j = 0; j < aCh.size(); ++j)  {
                            aCh[j]->RemoveParent(a);
                        }
                        aCh.clear();
                        aCh.push_back(MetaNode::GetZero());
                    }
                    else {
                        for (size_t j = 0; j < aCh.size(); ++j)  {
                            if (m == aCh[j].get()) {
	                            aCh[j]->RemoveParent(a);
                                aCh.erase(aCh.begin() +j);
                                break;
                            }
                        }
                    }
                    if (aCh.empty()) {
                        aCh.push_back(MetaNode::GetOne());
                    }
                }
            }
            else {
            // Normalize the node and promote its weights to the parents
	            double w = m->Normalize();
	            BOOST_FOREACH(ANDNode *a, m->GetParents()) {
	                a->ScaleWeight(w);
	            }
            }
        }
    }
    return 1.0;
}

// Sets each AND node of variables to marginalize to be the result of minimizing
// the respective MetaNode children of each AND node. Redundancy can be
// resolved outside.
ANDNodePtr NodeManager::Minimize(MetaNodePtr root, const Scope &s,
        const DirectedGraph &embeddedpt) {
    if (root.get() == MetaNode::GetZero().get()) {
//        return make_shared<ANDNode>(0.0, MetaNodeList(1, MetaNode::GetZero()));
        return ANDNodePtr(new ANDNode(0.0, MetaNodeList(1, MetaNode::GetZero())));
    }
    else if (root.get() == MetaNode::GetOne().get()) {
//        return make_shared<ANDNode>(1.0, MetaNodeList(1, MetaNode::GetOne()));
        return ANDNodePtr(new ANDNode(1.0, MetaNodeList(1, MetaNode::GetOne())));
    }
    int varid = root->GetVarID();
    int card = root->GetCard();

    int elimvar = s.GetOrdering().front();
#ifdef USE_OPCACHE
    Operation ocEntry(MIN, root, elimvar);
    OperationCache::iterator ocit = opCache.find(ocEntry);
    if ( ocit != opCache.end() ) {
        //Found result in cache
        cout << "operator: " << ocit->first.GetOperator() << endl;
        cout << "root: " << root.get() << endl;
        cout << "elimvar: " << elimvar << endl;
//        cout << "Returning: " << ocit->second.get() << endl;
        return ocit->second;
    }
#endif

    // Minimize each subgraph
    const vector<ANDNodePtr> &andNodes = root->GetChildren();
    vector<ANDNodePtr> newANDNodes;
    BOOST_FOREACH(ANDNodePtr i, andNodes) {
        const vector<MetaNodePtr> &metaNodes = i->GetChildren();
        vector<MetaNodePtr> newMetaNodes;
        bool terminalOnly = true;
        double weight = 1.0;
        BOOST_FOREACH(MetaNodePtr j, metaNodes) {
            ANDNodePtr newMetaNodeList = Minimize(j, s, embeddedpt);
            /*
            cout << "Input lhs: " << "(w="<< w << ", rhs size=" << paramSets[i].second.size() << ")"<< endl;
            paramSets[i].first->RecursivePrint(cout); cout << endl;
            cout << "SubDD:" << "(" << varid << "," << k << ")" << endl;
            subDD->RecursivePrint(cout); cout << endl;
            */

            // Note: would have to modify for other operators
            weight *= newMetaNodeList->GetWeight();
            BOOST_FOREACH(MetaNodePtr m, newMetaNodeList->GetChildren()) {
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
        newANDNodes.push_back(ANDNodePtr(new ANDNode(i->GetWeight() * weight, newMetaNodes)));
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
        ANDNodePtr newAND(new ANDNode(weight, newMetaNodes));
        for (unsigned int i = 0; i < andNodes.size(); ++i) {
            newANDNodes.push_back(newAND);
        }
    }
    /*
    Scope var;
    var.AddVar(varid, card);
    */
    ANDNodePtr ret = CreateMetaNode(varid, card, newANDNodes);
#ifdef USE_OPCACHE
    opCache.insert(make_pair<Operation, ANDNodePtr>(ocEntry, ret));
    opCacheMemUsage += (ocEntry.MemUsage() + sizeof(ret) + (ret->GetChildren().size() * sizeof(MetaNodePtr))) / MB_PER_BYTE;

    // Purge if op cache is too large
    if (opCacheMemUsage > OCMBLimit) {
        NodeManager::GetNodeManager()->PurgeOpCache();
    }

    if (opCacheMemUsage > maxOpCacheMemUsage) maxOpCacheMemUsage = opCacheMemUsage;
#endif
    /*
    cout << "Created cache entry(MAX)" << endl;
    cout << "elimvar:" << elimvar << endl;
    cout << "keys:";
    BOOST_FOREACH(size_t k, entryKey.GetParamSet()) {
        cout << " " << (void*)k;
    }
    cout << endl << "res:ANDNodePtrdl;
    */
    return ret;
}

double NodeManager::ConditionFast(MetaNodePtr root, const Assignment &s,
        const set<int> &relevantVars) {
    if (root.get() == MetaNode::GetZero().get()) {
        return 0.0;
    }
    else if (root.get() == MetaNode::GetOne().get()) {
        return 1.0;
    }

    // idea: do a dfs visit and store the nodes encountered
    queue<MetaNode*> q;
    vector<MetaNode*> visitOrder;
    set<MetaNode*> visited;

    q.push(root.get());
    while (!q.empty()) {
        MetaNode *c = q.front();
        visitOrder.push_back(c);
        q.pop();
        BOOST_FOREACH(ANDNodePtr a, c->GetChildren()) {
            BOOST_FOREACH(MetaNodePtr m, a->GetChildren()) {
                if (visited.find(m.get()) == visited.end() &&
                        relevantVars.find(m->GetVarID()) != relevantVars.end()) {
	                visited.insert(m.get());
	                q.push(m.get());
                }

            }
        }
    }

    for (int i = visitOrder.size() - 1; i >= 0; --i) {
        MetaNode *&m = visitOrder[i];
        int vid = m->GetVarID();
        if (s.VarExists(vid)) {
            double w = m->GetChildren()[s.GetVal(vid)]->GetWeight();
            /*
            if (i == 0) {
                assert(m == root.get());
                return w;
            }
            */
            // propagate weight directly to parents -- reassign children
            BOOST_FOREACH(ANDNode *a, m->GetParents()) {
                a->ScaleWeight(w);
                vector<MetaNodePtr> &aCh = a->GetChildren();
                for (size_t j = 0; j < aCh.size(); ++j) {
                    if (m == aCh[j].get()) {
                        if (w == 0) {
                            aCh.clear();
                            aCh.push_back(MetaNode::GetZero());
                        }
                        else {
                            aCh.erase(aCh.begin() + j);
                        }
                        break;
                    }
                }
                if (aCh.empty()) {
                    aCh.push_back(MetaNode::GetOne());
                }
            }
        }
        else {
            // Normalize the node and promote its weights to the parents
            // Check if m is now redundant
            if (m->IsRedundant()) {
                double w = m->GetChildren()[0]->GetWeight();
                BOOST_FOREACH(ANDNode *a, m->GetParents()) {
                    a->ScaleWeight(w);
                    vector<MetaNodePtr> &aCh = a->GetChildren();
                    MetaNodePtr mCh = m->GetChildren()[0]->GetChildren()[0];
                    if (mCh == MetaNode::GetZero()) {
                        for (size_t j = 0; j < aCh.size(); ++j)  {
                            aCh[j]->RemoveParent(a);
                        }
                        aCh.clear();
                        aCh.push_back(MetaNode::GetZero());
                    }
                    else {
                        for (size_t j = 0; j < aCh.size(); ++j)  {
                            if (m == aCh[j].get()) {
	                            aCh[j]->RemoveParent(a);
                                aCh.erase(aCh.begin() +j);
                                break;
                            }
                        }
                    }
                    if (aCh.empty()) {
                        aCh.push_back(MetaNode::GetOne());
                    }
                }
            }
            else {
	            double w = m->Normalize();
	            BOOST_FOREACH(ANDNode *a, m->GetParents()) {
	                a->ScaleWeight(w);
	            }
            }
        }
    }
    return 1.0;
}

ANDNodePtr NodeManager::Condition(MetaNodePtr root, const Assignment &cond) {
    if (root.get() == MetaNode::GetOne().get()) {
//        return make_shared<ANDNode>(1.0, MetaNodeList(1, root));
        return ANDNodePtr(new ANDNode(1.0, MetaNodeList(1, root)));
    }
    else if (root.get() == MetaNode::GetZero().get()) {
//        return make_shared<ANDNode>(0.0, MetaNodeList(1, root));
        return ANDNodePtr(new ANDNode(0.0, MetaNodeList(1, root)));
    }

    vector<ANDNodePtr> newANDNodes;
    const vector<ANDNodePtr> &rootCh = root->GetChildren();
    int val;
    if ( (val = cond.GetVal(root->GetVarID())) != ERROR_VAL ) {
        vector<MetaNodePtr> newMetaNodes;
        bool terminalOnly = true;
        double weight = 1.0;
        BOOST_FOREACH(MetaNodePtr j, rootCh[val]->GetChildren()) {
            ANDNodePtr newMetaNodeList = Condition(j, cond);

            // Note: would have to modify for other operators
            weight *= newMetaNodeList->GetWeight();
            BOOST_FOREACH(MetaNodePtr m, newMetaNodeList->GetChildren()) {
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
        newANDNodes.push_back(ANDNodePtr(new ANDNode(rootCh[val]->GetWeight() * weight, newMetaNodes)));
    }
    else {
        BOOST_FOREACH(ANDNodePtr i, root->GetChildren()) {
            vector<MetaNodePtr> newMetaNodes;
            double weight = 1.0;
            BOOST_FOREACH(MetaNodePtr j, i->GetChildren()) {
                ANDNodePtr newMetaNodeList = Condition(j, cond);
                bool terminalOnly = true;

                // Note: would have to modify for other operators
                weight *= newMetaNodeList->GetWeight();
                BOOST_FOREACH(MetaNodePtr m, newMetaNodeList->GetChildren()) {
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
            newANDNodes.push_back(ANDNodePtr(new ANDNode(i->GetWeight() * weight, newMetaNodes)));
        }
    }
    ANDNodePtr ret = CreateMetaNode(root->GetVarID(), root->GetCard(), newANDNodes);
    return ret;
}

inline ANDNodePtr NodeManager::LookupUT(ANDNodePtr &temp) {
    UniqueTable::iterator it = ut.find(temp->GetChildren()[0]);
    if (it != ut.end()) {
//        return make_shared<ANDNode>(temp->GetWeight(), MetaNodeList(1, *it));
        return ANDNodePtr(new ANDNode(temp->GetWeight(), MetaNodeList(1, *it)));
    }
    else {
	    // Confirmed uniqueness, add AND nodes of new node to the parent
        // lists of their children
	    BOOST_FOREACH(ANDNodePtr a, temp->GetChildren()[0]->GetChildren()) {
	        BOOST_FOREACH(MetaNodePtr mm, a->GetChildren()) {
	            mm->AddParent(a.get());
	        }
	    }
	    temp->GetChildren()[0]->SetChildrenParent(temp->GetChildren()[0]);

//	    temp->GetChildren()[0]->AddParent(temp);
        ut.insert(temp->GetChildren()[0]);
        utMemUsage += temp->GetChildren()[0]->MemUsage() / MB_PER_BYTE;
        if (utMemUsage > maxUTMemUsage) maxUTMemUsage = utMemUsage;
        return temp;
    }
}

double NodeManager::MemUsage() const {
    double memUsage = 0;
    BOOST_FOREACH(MetaNodePtr m, ut) {
        memUsage += m->MemUsage();
    }
    return (sizeof(NodeManager) + memUsage) / MB_PER_BYTE;
}

double NodeManager::OpCacheMemUsage() const {
    double memUsage = 0;
    OperationCache::const_iterator it = opCache.begin();
    for (; it != opCache.end(); ++it) {
//    BOOST_FOREACH(OperationCache::value_type i, opCache) {
        memUsage += sizeof(*it) + it->first.MemUsage();
        memUsage += sizeof(it->second);
        memUsage += sizeof(MetaNodePtr) * it->second->GetChildren().size();
    }
    return (sizeof(opCache) + memUsage) / MB_PER_BYTE;
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
        out << i.get() << ":" << i->refs << endl;
    }
}

bool NodeManager::initialized = false;
NodeManager *NodeManager::singleton = NULL;

} // end of aomdd namespace
