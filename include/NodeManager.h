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

typedef boost::unordered_multiset<MetaNode*> ParamSet;

class Operation {
    Operator op;
    ParamSet params;
    int varid;
    size_t hashVal;
public:
    Operation() :
        op(NONE) {
    }
    Operation(Operator o, MetaNodePtr arg, int vid = 0) :
        op(o), varid(vid) {
        params.insert(arg.get());
        hashVal = hash_value(*this);
    }
    Operation(Operator o, MetaNodePtr arg1, const std::vector<MetaNodePtr> &arg2) :
        op(o) {
        params.insert(arg1.get());
        for (unsigned int i = 0; i < arg2.size(); ++i) {
            params.insert(arg2[i].get());
        }
        hashVal = hash_value(*this);
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
        memUsage += sizeof(Operation) +
                (params.size() * sizeof(ParamSet::value_type));
        return memUsage;
    }

    inline size_t GetStoredHash() const {
        return hashVal;
    }

    friend inline std::size_t hash_value(const Operation &o) {
        size_t seed = 0;
        boost::hash_combine(seed, o.GetOperator());
        boost::hash_combine(seed, o.GetVarID());
        BOOST_FOREACH(ParamSet::value_type i, o.GetParamSet()) {
            boost::hash_combine(seed, i);
        }
        return seed;
    }
};


struct nodehasher {
    std::size_t operator()(const MetaNodePtr &node) const {
        return node->GetStoredHash();
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
        return o.GetStoredHash();
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

#define USE_SPARSE

#ifdef USE_SPARSE
typedef google::sparse_hash_set<MetaNodePtr, nodehasher, eqnode> UniqueTable;
typedef google::sparse_hash_map<Operation, WeightedMetaNodeList, ophasher, eqop> OperationCache;
#else
typedef google::dense_hash_set<MetaNodePtr, nodehasher, eqnode> UniqueTable;
typedef google::dense_hash_map<Operation, WeightedMetaNodeList, ophasher, eqop> OperationCache;
#endif


//typedef boost::unordered_set<MetaNodePtr, nodehasher, eqnode> UniqueTable;
//typedef boost::unordered_map<Operation, MetaNodePtr> OperationCache;
typedef std::pair<MetaNodePtr, std::vector<MetaNodePtr> > ApplyParamSet;

class NodeManager {
    UniqueTable ut;
    OperationCache opCache;

    double utMemUsage;
    double maxUTMemUsage;
    double opCacheMemUsage;
    double maxOpCacheMemUsage;

    double MBLimit;

    NodeManager() {
#ifndef USE_SPARSE
        MetaNodePtr nullKey(new MetaNode(-1, 0, std::vector<ANDNodePtr>()));
        ut.set_empty_key(nullKey);
        Operation nullOpKey;
        opCache.set_empty_key(nullOpKey);
#endif
        MetaNodePtr delKey(new MetaNode(-10, 0, std::vector<ANDNodePtr>()));
        ut.set_deleted_key(delKey);

        utMemUsage = MemUsage();
        maxUTMemUsage = MemUsage();
        opCacheMemUsage = OpCacheMemUsage();
        maxOpCacheMemUsage = OpCacheMemUsage();
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

    inline WeightedMetaNodeList LookupUT(WeightedMetaNodeList &temp) {
        UniqueTable::iterator it = ut.find(temp.first[0]);
        if (it != ut.end()) {
            return WeightedMetaNodeList(MetaNodeList(1, *it), temp.second);
        }
        else {
            ut.insert(temp.first[0]);
            utMemUsage += temp.first[0]->MemUsage() / pow(2.0, 20);
            if (utMemUsage > maxUTMemUsage) maxUTMemUsage = utMemUsage;
            return temp;
        }
    }

    inline void SetMBLimit(double m) { MBLimit = m; }
    inline double GetMBLimit() { return MBLimit; }

    inline double MemUsage() const {
        double memUsage = 0;
        BOOST_FOREACH(MetaNodePtr m, ut) {
            memUsage += m->MemUsage();
        }
        return (sizeof(ut) + memUsage) / pow(2.0,20);
    }

    inline void UTGarbageCollect() {
        UniqueTable::iterator it = ut.begin();
        bool done = false;
        while (!done) {
            done = true;
            for (; it != ut.end(); ++it) {
                if (it->use_count() == 1) {
                    done = false;
                    ut.erase(it);
                }
            }
            if (!done) it = ut.begin();
        }
        utMemUsage = MemUsage();
    }

    inline double OpCacheMemUsage() const {
        double memUsage = 0;
        BOOST_FOREACH(OperationCache::value_type i, opCache) {
            memUsage += sizeof(i) + i.first.MemUsage();
            memUsage += sizeof(i.second);
            memUsage += sizeof(MetaNodePtr) * i.second.first.size();
        }
        return (sizeof(opCache) + memUsage) / pow(2.0,20);
    }

    inline void PurgeOpCache() {
        opCache.clear();
        opCacheMemUsage = OpCacheMemUsage();
    }

    inline double GetUTMemUsage() const { return utMemUsage; }
    inline double GetOCMemUsage() const { return opCacheMemUsage; }
    inline double GetMaxUTMemUsage() const { return maxUTMemUsage; }
    inline double GetMaxOCMemUsage() const { return maxOpCacheMemUsage; }

};

} // end of aomdd namespace

#endif /* NODEMANAGER_H_ */
