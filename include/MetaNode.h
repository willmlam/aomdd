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
#include <boost/foreach.hpp>

namespace aomdd {

// tolerance for equal weights
const double TOLERANCE = 1e-50;

class MetaNode {
friend class NodeManager;
public:
    typedef boost::intrusive_ptr<MetaNode> MetaNodePtr;

    // Used to represent children of the metanode
    // Corresponds to weights of assignments
    class ANDNode {
        MetaNodePtr parent;
        double weight;
        std::vector<MetaNodePtr> children;
    public:
        mutable long refs;

        ANDNode();
        virtual ~ANDNode();

        ANDNode(double w, const std::vector<MetaNodePtr> &ch);

        double GetWeight() const;
        void SetWeight(double w);
        void ScaleWeight(double w);

        void SetChildren(const std::vector<MetaNodePtr> &ch);

        inline void SetParent(MetaNodePtr p) { parent = p; }

        inline std::vector<MetaNodePtr> &GetChildren() { return children; }
        inline const std::vector<MetaNodePtr> &GetChildren() const { return children; }

        double Evaluate(const Assignment &a);

        bool operator==(const ANDNode &rhs) const;

        void Save(std::ostream &out, std::string prefix = "") const;
        void RecursivePrint(std::ostream &out, std::string prefix) const;
        void RecursivePrint(std::ostream &out) const;
        void GenerateDiagram(DirectedGraph &diagram, const DVertexDesc &parent) const;

        inline size_t MemUsage() const {
            return sizeof(ANDNode) + (children.size() * sizeof(MetaNodePtr));
        }
    };

    typedef boost::intrusive_ptr<ANDNode> ANDNodePtr;

private:
    // IDs handled by shared_ptr wrappers
    // Scope should contain only one variable, or if scope is empty scope,
    // this node is a terminal
    // Pointer due to plan to use a set of common Scope objects for all nodes
    // of the same variable
    int varID;
    unsigned int card;
    // children: ANDNodes
    std::set<ANDNode*> parents;
    std::vector<ANDNodePtr> children;
//    double weight;

    size_t hashVal;


    // Used to make terminal nodes singletons
    static bool zeroInit;
    static bool oneInit;
    static MetaNodePtr terminalZero;
    static MetaNodePtr terminalOne;

    double cachedElimValue;
    bool elimValueCached;

    void FindUniqueNodes(boost::unordered_set<const MetaNode *> &nodeSet) const;
    void FindUniqueNodes(boost::unordered_set<const MetaNode *> &nodeSet,
            std::vector<unsigned int> &numMeta) const;

public:
    mutable long refs;

    MetaNode();

    virtual ~MetaNode();

    MetaNode(const Scope &var, const std::vector<ANDNodePtr> &ch);
    MetaNode(int varidIn, int cardIn, const std::vector<ANDNodePtr> &ch);

    inline int GetVarID() const { return varID; }

    inline unsigned int GetCard() const { return card; }
/*
    inline double GetWeight() const { return weight; }

    inline void SetWeight(double w) { weight = w; }
    */

    inline std::vector<ANDNodePtr> &GetChildren() { return children; }
    inline const std::vector<ANDNodePtr> &GetChildren() const { return children; }
    inline void SetChildren(const std::vector<ANDNodePtr> &ch) { children = ch; }

    void SetChildrenParent(MetaNodePtr m);
    inline void AddParent(ANDNode *a) {
        if (!IsTerminal()) {
            parents.insert(a);
        }
    }
    inline void RemoveParent(ANDNode *a) {
        parents.erase(a);
    }
    inline std::set<ANDNode*> &GetParents() { return parents; }
    inline const std::set<ANDNode*> &GetParents() const { return parents; }

    inline bool IsDummy() const { return card == 1; }

    inline bool IsTerminal() const { return this == GetZero().get() || this == GetOne().get(); }

    bool IsRedundant() const;

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
    DirectedGraph GenerateDiagram() const;

    void GenerateDiagram(DirectedGraph &diagram, const DVertexDesc &parent) const;

    friend inline size_t hash_value(const MetaNode &node) {
        size_t seed = 0;
        boost::hash_combine(seed, node.varID);
        boost::hash_combine(seed, node.card);
        BOOST_FOREACH(const ANDNodePtr &i, node.children) {
            boost::hash_combine(seed, i->GetWeight());
            BOOST_FOREACH(const MetaNodePtr &j, i->GetChildren()) {
                boost::hash_combine(seed, j.get());
            }
        }
        return seed;

    }

    inline static const MetaNodePtr &GetZero() {
        if (!zeroInit) {
            terminalZero = MetaNodePtr(new MetaNode());
            terminalZero->varID = -2;
//            terminalZero->weight = 0;
            zeroInit = true;
            return terminalZero;
        }
        else {
            return terminalZero;
        }
    }
    inline static const MetaNodePtr &GetOne() {
        if (!oneInit) {
            terminalOne = MetaNodePtr(new MetaNode());
            oneInit = true;
            return terminalOne;
        }
        else {
            return terminalOne;
        }
    }

    double ComputeTotalMemory() const;

    inline size_t MemUsage() const {
        size_t temp = sizeof(MetaNode) + (children.size() * sizeof(ANDNodePtr));
        BOOST_FOREACH(const ANDNodePtr &i, children) {
            temp += i->MemUsage();
        }
        temp += parents.size() * sizeof(ANDNode*);
        return temp;
    }

};

typedef MetaNode::MetaNodePtr MetaNodePtr;
typedef MetaNode::ANDNodePtr ANDNodePtr;

typedef MetaNode::ANDNode ANDNode;

inline void intrusive_ptr_add_ref(MetaNode *m) {
//    std::cout << "Increasing refcount" << std::endl;
    ++m->refs;
}
inline void intrusive_ptr_release(MetaNode *m) {
//    std::cout << "Decreasing refcount" << std::endl;
    if(--m->refs == 0) {
//        std::cout << "Releasing meta " << m << std::endl;
        delete m;
    }
}

inline void intrusive_ptr_add_ref(ANDNode *a) {
    ++a->refs;
}
inline void intrusive_ptr_release(ANDNode *a) {
    if(--a->refs == 0) {
//        std::cout << "Releasing " << a << std::endl;
        delete a;
    }
}

bool operator==(const ANDNodePtr &lhs, const ANDNodePtr &rhs);
bool operator!=(const ANDNodePtr &lhs, const ANDNodePtr &rhs);

} // end of aomdd namespace

#endif
