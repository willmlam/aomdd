/*
 *  MetaNode.h
 *  aomdd
 *
 *  Created by William Lam on 3/23/11.
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

// Note these classes do not own the pointers themselves, the node manager does.

#ifndef _METANODE_H_
#define _METANODE_H_

#include "base.h"
#include "graphbase.h"
#include "Scope.h"

namespace aomdd {

// tolerance for equal weights
const double TOLERANCE = 1e-50;

class MetaNode {
friend class NodeManager;
public:
    typedef boost::shared_ptr<MetaNode> MetaNodePtr;

    // Used to represent children of the metanode
    // Corresponds to weights of assignments
    class ANDNode {
        double weight;
        unsigned int numChildren;
        MetaNode **children;
    public:
        ANDNode();
        virtual ~ANDNode();

        ANDNode(double w, unsigned int nc, MetaNode **ch);

        double GetWeight() const;
        void SetWeight(double w);

        inline unsigned int GetNumChildren() const { return numChildren; }
        inline MetaNode **GetChildren() const { return children; }

        double Evaluate(const Assignment &a);

        bool operator==(const ANDNode &rhs) const;

        void Save(std::ostream &out, std::string prefix = "") const;
        void RecursivePrint(std::ostream &out, std::string prefix) const;
        void RecursivePrint(std::ostream &out) const;

        inline size_t MemUsage() const {
            return sizeof(ANDNode) + (numChildren * sizeof(MetaNode*));
        }
    };

//    typedef boost::shared_ptr<ANDNode> ANDNodePtr;

private:
    // IDs handled by shared_ptr wrappers
    // Scope should contain only one variable, or if scope is empty scope,
    // this node is a terminal
    // Pointer due to plan to use a set of common Scope objects for all nodes
    // of the same variable
    int varID;
    unsigned int card;
    // children: ANDNodes
//    std::vector<ANDNodePtr> children;
    ANDNode **children;
//    double weight;

    size_t hashVal;

    // Used to make terminal nodes singletons
    static bool zeroInit;
    static bool oneInit;
    static MetaNode *terminalZero;
    static MetaNode *terminalOne;

    double cachedElimValue;
    bool elimValueCached;

    void FindUniqueNodes(boost::unordered_set<const MetaNode *> &nodeSet) const;
    void FindUniqueNodes(boost::unordered_set<const MetaNode *> &nodeSet,
            std::vector<unsigned int> &numMeta) const;

public:
    MetaNode();

    virtual ~MetaNode();

    MetaNode(const Scope &var, ANDNode **ch);
    MetaNode(int varidIn, int cardIn, ANDNode **ch);

    inline int GetVarID() const { return varID; }

    inline unsigned int GetCard() const { return card; }
/*
    inline double GetWeight() const { return weight; }

    inline void SetWeight(double w) { weight = w; }
    */

    /*
    inline const std::vector<ANDNodePtr> &GetChildren() const { return children; }
    inline void SetChildren(const std::vector<ANDNodePtr> &ch) { children = ch; }
    */

    inline ANDNode **GetChildren() const { return children; }

    inline bool IsTerminal() const { return this == GetZero() || this == GetOne(); }

    inline size_t GetStoredHash() const { return hashVal; }

    // Normalizes below, sets weight and returns normalization constant
    double Normalize();

    // Returns the maximum value that the function rooted at this
    // metanode can take on, subject to the assignment
    double Maximum(const Assignment &a);

    // Returns the sum of the values of the function rooted at this
    // metanode, subject to the assignment
    double Sum(const Assignment &a);

    double Evaluate(const Assignment &a) const;

    void ClearElimCacheValue() { elimValueCached = false; }

//    bool operator==(const MetaNode &rhs) const;
    void Save(std::ostream &out, std::string prefix = "") const;
    void RecursivePrint(std::ostream &out, std::string prefix) const;
    void RecursivePrint(std::ostream &out) const;

    // Returns a pair (numMeta, numAND)
    std::pair<unsigned int, unsigned int> NumOfNodes() const;

    // Return a vector specifying the number of meta nodes for each variable
    // (Result is returned by reference)
    void GetNumNodesPerVar(std::vector<unsigned int> &numMeta) const;

    friend size_t hash_value(const MetaNode &node);

    inline static MetaNode *GetZero() {
        if (!zeroInit) {
            terminalZero = new MetaNode();
            terminalZero->varID = -2;
//            terminalZero->weight = 0;
            zeroInit = true;
            return terminalZero;
        }
        else {
            return terminalZero;
        }
    }
    inline static MetaNode *GetOne() {
        if (!oneInit) {
            terminalOne = new MetaNode();
            oneInit = true;
            return terminalOne;
        }
        else {
            return terminalOne;
        }
    }

    double ComputeTotalMemory() const;

    inline size_t MemUsage() const {
        size_t temp = sizeof(MetaNode) + (card * sizeof(ANDNode*));
        for (unsigned int i = 0; i < card; ++i) {
            temp += children[i]->MemUsage();
        }
        return temp;
    }

};

//typedef MetaNode::MetaNodePtr MetaNodePtr;
//typedef MetaNode::ANDNodePtr ANDNodePtr;
typedef MetaNode::ANDNode ANDNode;

} // end of aomdd namespace

#endif
