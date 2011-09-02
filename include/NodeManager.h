/*
 *  NodeManager.h
 *  aomdd
 *
 *  Created by William Lam on May 11, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#ifndef NODEMANAGER_H_
#define NODEMANAGER_H_

#include "base.h"
#include "graphbase.h"
#include "MetaNode.h"

namespace aomdd {

enum Operator {
    PROD, SUM, MAX, REDUCE, MARGINALIZE, NONE
};

typedef boost::unordered_multiset<size_t> ParamSet;

class Operation {
    Operator op;
    ParamSet params;
    int varid;

public:
    Operation() :
        op(NONE) {
    }
    Operation(Operator o, MetaNodePtr arg, int vid = 0) :
        op(o), varid(vid) {
        params.insert(size_t(arg.get()));
    }
    Operation(Operator o, MetaNodePtr arg1, const std::vector<MetaNodePtr> &arg2) :
        op(o) {
        params.insert(size_t(arg1.get()));
        for (unsigned int i = 0; i < arg2.size(); ++i) {
            params.insert(size_t(arg2[i].get()));
        }
    }

    inline const Operator &GetOperator() const {
        return op;
    }

    inline const ParamSet &GetParamSet() const {
        return params;
    }

    inline int GetVarID() const {
        return varid;
    }

    inline double MemUsage() const {
        double memUsage = 0;
        memUsage += sizeof(op) + sizeof(params) + sizeof(varid);
        BOOST_FOREACH(ParamSet::value_type m, params) {
            memUsage += sizeof(m);
        }
        return memUsage;
    }
};

size_t hash_value(const Operation &o);
bool operator==(const Operation &lhs, const Operation &rhs);

struct nodehasher {
    std::size_t operator()(const MetaNodePtr &node) const {
        size_t seed = 0;
        boost::hash_combine(seed, node->GetVarID());
        boost::hash_combine(seed, node->GetCard());
        BOOST_FOREACH(const ANDNodePtr &i, node->GetChildren()) {
            boost::hash_combine(seed, i->GetWeight());
            BOOST_FOREACH(const MetaNodePtr &j, i->GetChildren()) {
                boost::hash_combine(seed, j.get());
            }
        }
        return seed;
    }
};

struct eqnode {
    bool operator()(const MetaNodePtr &lhs, const MetaNodePtr &rhs) const {
        if (lhs.get() == MetaNode::GetZero().get() && rhs.get()
                == MetaNode::GetOne().get())
            return false;
        if (rhs.get() == MetaNode::GetZero().get() && lhs.get()
                == MetaNode::GetOne().get())
            return false;
        if (lhs->GetStoredHash() != rhs->GetStoredHash()) {
            return false;
        }
        if (lhs->GetVarID() != rhs->GetVarID() || lhs->GetCard() != rhs->GetCard()
                || lhs->GetChildren().size() != rhs->GetChildren().size())
            return false;
        for (unsigned int i = 0; i < lhs->GetChildren().size(); i++) {
            if (lhs->GetChildren()[i] != rhs->GetChildren()[i])
                return false;
        }
        return true;
    }
};

struct ophasher {
    std::size_t operator()(const Operation &o) const {
        size_t seed = 0;
        boost::hash_combine(seed, o.GetOperator());
        boost::hash_combine(seed, o.GetVarID());
        BOOST_FOREACH(ParamSet::value_type i, o.GetParamSet()) {
            boost::hash_combine(seed, i);
        }
        return seed;
    }
};

struct eqop {
    bool operator()(const Operation &lhs, const Operation &rhs) const {
        return lhs.GetOperator() == rhs.GetOperator() &&
                lhs.GetVarID() == rhs.GetVarID() &&
                lhs.GetParamSet() == rhs.GetParamSet();
    }
};

typedef std::vector<MetaNodePtr> MetaNodeList;
typedef std::pair<MetaNodeList, double> WeightedMetaNodeList;

//typedef boost::unordered_set<MetaNodePtr, nodehasher, eqnode> UniqueTable;
typedef google::dense_hash_set<MetaNodePtr, nodehasher, eqnode> UniqueTable;
//typedef google::sparse_hash_set<MetaNodePtr, nodehasher, eqnode> UniqueTable;
//typedef boost::unordered_map<Operation, MetaNodePtr> OperationCache;
typedef google::dense_hash_map<Operation, WeightedMetaNodeList, ophasher, eqop> OperationCache;
//typedef google::sparse_hash_map<Operation, WeightedMetaNodeList, ophasher, eqop> OperationCache;
typedef std::pair<MetaNodePtr, std::vector<MetaNodePtr> > ApplyParamSet;

class NodeManager {
    UniqueTable ut;
    OperationCache opCache;
    NodeManager() {
        MetaNodePtr nullKey(new MetaNode(-1, 0, std::vector<ANDNodePtr>()));
        ut.set_empty_key(nullKey);
        Operation nullOpKey;
        opCache.set_empty_key(nullOpKey);
    }
    NodeManager(NodeManager const&) {
    }
    NodeManager& operator=(NodeManager const&) {
        return *this;
    }

    static bool initialized;
    static NodeManager *singleton;

    MetaNodePtr NormalizeHelper(MetaNodePtr root);

public:
    static NodeManager *GetNodeManager();
    // Create a metanode from a variable with a children list
    WeightedMetaNodeList CreateMetaNode(const Scope &var,
            const std::vector<ANDNodePtr> &ch);

    WeightedMetaNodeList CreateMetaNode(int varid, unsigned int card,
            const std::vector<ANDNodePtr> &ch);

    // Create a metanode based on a tabular form of the function
    // Variable ordering is defined by the scope
    WeightedMetaNodeList CreateMetaNode(const Scope &vars,
            const std::vector<double> &vals);

    /*
    // Be sure the input node is a root!
    // Returns a vector of pointers since ANDNodes can have multiple
    // MetaNode children
    std::vector<MetaNodePtr> FullReduce(MetaNodePtr node, double &w, bool isRoot=false);

    // Driver function that returns a dummy root if needed (to combine
    // multiple MetaNodes as a single output
    MetaNodePtr FullReduce(MetaNodePtr node);
    */

    // Same as above, but single level version, it assumes all the decision
    // diagrams rooted by the children are already fully reduced
    WeightedMetaNodeList SingleLevelFullReduce(MetaNodePtr node);

    WeightedMetaNodeList Apply(MetaNodePtr lhs, const std::vector<MetaNodePtr> &rhs, Operator op,
            const DirectedGraph &embeddedPT);

    std::vector<ApplyParamSet> GetParamSets(const DirectedGraph &tree,
            const std::vector<MetaNodePtr> &lhs,
            const std::vector<MetaNodePtr> &rhs) const;

    WeightedMetaNodeList Marginalize(MetaNodePtr root, const Scope &s, const DirectedGraph &embeddedPT, bool &sumOpPerformed);
    WeightedMetaNodeList Maximize(MetaNodePtr root, const Scope &s, const DirectedGraph &embeddedPT);
    WeightedMetaNodeList Condition(MetaNodePtr root, const Assignment &cond);

    // Normalizes the weights of the immediate AND nodes to sum to 1.
//    double Normalize(MetaNodePtr root);


    inline unsigned int GetNumberOfNodes() const {
        return ut.size();
    }
    inline unsigned int GetNumberOfANDNodes() const {
        unsigned int count = 0;
        BOOST_FOREACH(MetaNodePtr m, ut) {
            count += m->GetCard();
        }
        return count;
    }

    inline unsigned int GetNumberOfOpCacheEntries() const {
        return opCache.size();
    }

    inline size_t utBucketCount() const { return ut.bucket_count(); }
    inline size_t ocBucketCount() const { return opCache.bucket_count(); }

    inline void PrintUTBucketSizes() const {
        for (size_t i = 0; i < ut.bucket_count(); ++i) {
            std::cout << i << "\t" << ut.bucket_size(i) << std::endl;
        }
    }

    void PrintUniqueTable(std::ostream &out) const;
    void PrintReferenceCount(std::ostream &out) const;

    inline double MemUsage() const {
        double memUsage = 0;
        BOOST_FOREACH(MetaNodePtr m, ut) {
            memUsage += m->MemUsage();
        }
        return memUsage / pow(2.0,20);
    }

    inline double OpCacheMemUsage() const {
        double memUsage = 0;
        BOOST_FOREACH(OperationCache::value_type i, opCache) {
            memUsage += sizeof(i.first) + i.first.MemUsage();
            memUsage += sizeof(i.second.first) + sizeof(i.second.second);
            BOOST_FOREACH(MetaNodePtr m, i.second.first) {
                memUsage += sizeof(m);
            }
        }
        return memUsage / pow(2.0,20);
    }

};

} // end of aomdd namespace

#endif /* NODEMANAGER_H_ */
