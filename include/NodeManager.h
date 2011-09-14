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
    Operation(Operator o, MetaNode* arg, int vid = 0) :
        op(o), varid(vid) {
        params.insert(size_t(arg));
    }
    Operation(Operator o, MetaNode* arg1, const std::vector<MetaNode*> &arg2) :
        op(o) {
        params.insert(size_t(arg1));
        for (unsigned int i = 0; i < arg2.size(); ++i) {
            params.insert(size_t(arg2[i]));
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
    std::size_t operator()(const MetaNode *node) const {
        return node->GetStoredHash();
    }
};

struct eqnode {
    bool operator()(const MetaNode *lhs, const MetaNode *rhs) const {
        if ( (lhs == MetaNode::GetZero() && rhs == MetaNode::GetOne()) ||
                (rhs == MetaNode::GetZero() && lhs == MetaNode::GetOne()) )
            return false;
        if (lhs->GetStoredHash() != rhs->GetStoredHash()) {
            return false;
        }
        if (lhs->GetVarID() != rhs->GetVarID() || lhs->GetCard() != rhs->GetCard())
            return false;
        for (unsigned int i = 0; i < lhs->GetCard(); i++) {
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

typedef std::pair<unsigned int, MetaNode **> MetaNodeList;
typedef std::pair<MetaNodeList, double> WeightedMetaNodeList;

#define USE_SPARSE

#ifdef USE_SPARSE
typedef google::sparse_hash_set<MetaNode*, nodehasher, eqnode> UniqueTable;
typedef google::sparse_hash_map<Operation, WeightedMetaNodeList, ophasher, eqop> OperationCache;
#else
typedef google::dense_hash_set<MetaNode*, nodehasher, eqnode> UniqueTable;
typedef google::dense_hash_map<Operation, WeightedMetaNodeList, ophasher, eqop> OperationCache;
#endif


//typedef boost::unordered_set<MetaNodePtr, nodehasher, eqnode> UniqueTable;
//typedef boost::unordered_map<Operation, MetaNodePtr> OperationCache;
typedef std::pair<MetaNode*, std::vector<MetaNode*> > ApplyParamSet;

class NodeManager {
    UniqueTable ut;
    OperationCache opCache;

    // Used for intermediate operations. Cleared when finished each time
    UniqueTable utTemp;
    OperationCache opCacheTemp;

    bool useTempMode;
    NodeManager() {
#ifndef USE_SPARSE
        MetaNodePtr nullKey(new MetaNode(-1, 0, std::vector<ANDNodePtr>()));
        ut.set_empty_key(nullKey);
        utTemp.set_empty_key(nullKey);
        Operation nullOpKey;
        opCache.set_empty_key(nullOpKey);
        opCacheTemp.set_empty_key(nullOpKey);
#endif
    }
    NodeManager(NodeManager const&) : useTempMode(false) {
    }
    NodeManager& operator=(NodeManager const&) {
        return *this;
    }

    static bool initialized;
    static NodeManager *singleton;

    MetaNode *NormalizeHelper(MetaNode *root);

public:
    static NodeManager *GetNodeManager();
    // Create a metanode from a variable with a children list
    WeightedMetaNodeList CreateMetaNode(const Scope &var,
            ANDNode **ch);

    WeightedMetaNodeList CreateMetaNode(int varid, unsigned int card,
            ANDNode **ch);

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
    WeightedMetaNodeList SingleLevelFullReduce(MetaNode *node);

    WeightedMetaNodeList Apply(MetaNode *lhs, const std::vector<MetaNode *> &rhs, Operator op,
            const DirectedGraph &embeddedPT);

    std::vector<ApplyParamSet> GetParamSets(const DirectedGraph &tree,
            const std::vector<MetaNode *> &lhs,
            const std::vector<MetaNode *> &rhs) const;

    WeightedMetaNodeList Marginalize(MetaNode *root, const Scope &s, const DirectedGraph &embeddedPT, bool &sumOpPerformed);
    WeightedMetaNodeList Maximize(MetaNode *root, const Scope &s, const DirectedGraph &embeddedPT);
    WeightedMetaNodeList Condition(MetaNode *root, const Assignment &cond);

    // Normalizes the weights of the immediate AND nodes to sum to 1.
//    double Normalize(MetaNodePtr root);


    inline unsigned int GetNumberOfNodes() const {
        return ut.size();
    }
    inline unsigned int GetNumberOfANDNodes() const {
        unsigned int count = 0;
        BOOST_FOREACH(MetaNode *m, ut) {
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

    inline void SetTempMode(bool mode) {
        useTempMode = mode;
        if (!useTempMode) {
            utTemp.clear();
            opCacheTemp.clear();
        }
    }

    inline WeightedMetaNodeList LookupUT(WeightedMetaNodeList &temp) {
        UniqueTable::iterator it = ut.find(temp.first.second[0]);
        if (it != ut.end()) {
            MetaNode **nodelist(new MetaNode*[1]);
            nodelist[0] = *it;
            return WeightedMetaNodeList(MetaNodeList(1, nodelist), temp.second);
        }
        else {
            if (useTempMode) {
                it = utTemp.find(temp.first.second[0]);
                if (it != utTemp.end()) {
                    MetaNode **nodelist(new MetaNode*[1]);
                    nodelist[0] = *it;
                    return WeightedMetaNodeList(MetaNodeList(1, nodelist), temp.second);
                }
                else {
                    utTemp.insert(temp.first.second[0]);
                    return temp;
                }
            }
            ut.insert(temp.first.second[0]);
            return temp;
        }
    }

    inline double MemUsage() const {
        double memUsage = 0;
        BOOST_FOREACH(MetaNode *m, ut) {
            memUsage += m->MemUsage() + sizeof(m);
        }
        return (sizeof(ut) + memUsage) / pow(2.0,20);
    }

    inline double OpCacheMemUsage() const {
        double memUsage = 0;
        BOOST_FOREACH(OperationCache::value_type i, opCache) {
            memUsage += sizeof(i.first) + i.first.MemUsage();
            memUsage += sizeof(i.second.first.first) + sizeof(i.second.first.second);
            unsigned int nc = i.second.first.first;
            for (unsigned int j = 0; j < nc; ++j) {
                MetaNode *m = i.second.first.second[j];
                memUsage += sizeof(m);
            }
        }
        return (sizeof(opCache) + memUsage) / pow(2.0,20);
    }

};

} // end of aomdd namespace

#endif /* NODEMANAGER_H_ */
