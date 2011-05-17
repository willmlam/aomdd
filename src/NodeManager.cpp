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
    BOOST_FOREACH(ParamSet::value_type i, o.GetParamSet())
                {
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

MetaNodePtr NodeManager::CreateMetaNode(const Scope &var,
        const vector<ANDNodePtr> &ch) {
    MetaNodePtr temp(new MetaNode(var, ch));
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
        const vector<ANDNodePtr> &ch) {
    Scope var;
    var.AddVar(varid, card);
    return CreateMetaNode(var, ch);
}

MetaNodePtr NodeManager::CreateMetaNode(const Scope &vars,
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
        vector<vector<double> > valParts = SplitVector(vals, card);
        for (unsigned int i = 0; i < card; i++) {
            vector<MetaNodePtr> ANDch;
            ANDch.push_back(CreateMetaNode(restVars, valParts[i]));
            ANDNodePtr newNode(new MetaNode::ANDNode(1.0, ANDch));
            children.push_back(newNode);
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
    MetaNodePtr ret(CreateMetaNode(rootVar, children));
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
            ret.insert(ret.begin(), reduceSet.begin(), reduceSet.end());
        }
        w = temp->GetWeight();
        return ret;
    }
    else {
        ut.erase(node);
        vector<ANDNodePtr> newMetaCh;
        for (unsigned int i = 0; i < ch.size(); ++i) {
            vector<MetaNodePtr> andCh = ch[i]->GetChildren();
            vector<MetaNodePtr> newandCh;
            for (unsigned int j = 0; j < andCh.size(); ++j) {
                vector<MetaNodePtr> reduceSet = FullReduce(andCh[j], w);
                newandCh.insert(newandCh.begin(), reduceSet.begin(),
                        reduceSet.end());
            }
            ANDNodePtr
                    rAND(new MetaNode::ANDNode(w * ch[i]->GetWeight(), newandCh));
            newMetaCh.push_back(rAND);
            w = 1.0;
        }
        MetaNodePtr newMeta = CreateMetaNode(node->GetVarID(), node->GetCard(),
                newMetaCh);
        return vector<MetaNodePtr>(1, newMeta);
    }

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
