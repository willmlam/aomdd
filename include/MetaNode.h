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
        std::vector<MetaNodePtr> children;
    public:
        ANDNode();
        virtual ~ANDNode();

        ANDNode(double w, const std::vector<MetaNodePtr> &ch);

        double GetWeight() const;
        void SetWeight(double w);

        void SetChildren(const std::vector<MetaNodePtr> &ch);

        const std::vector<MetaNodePtr> &GetChildren() const;

        double Evaluate(const Assignment &a);

        bool operator==(const ANDNode &rhs) const;

        void Save(std::ostream &out, std::string prefix = "") const;
        void RecursivePrint(std::ostream &out, std::string prefix) const;
        void RecursivePrint(std::ostream &out) const;
        void GenerateDiagram(DirectedGraph &diagram, const DVertexDesc &parent) const;

        inline size_t MemUsage() const {
            size_t mtpSize = sizeof(MetaNodePtr) + sizeof(size_t);
            return sizeof(weight) + sizeof(children) + (children.size() * mtpSize);
        }
    };

    typedef boost::shared_ptr<ANDNode> ANDNodePtr;

private:
    // IDs handled by shared_ptr wrappers
    // Scope should contain only one variable, or if scope is empty scope,
    // this node is a terminal
    // Pointer due to plan to use a set of common Scope objects for all nodes
    // of the same variable
    int varID;
    unsigned int card;
    // children: ANDNodes
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

public:
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

    inline const std::vector<ANDNodePtr> &GetChildren() const { return children; }
    inline void SetChildren(const std::vector<ANDNodePtr> &ch) { children = ch; }

    inline bool IsDummy() const { return card == 1; }

    inline bool IsTerminal() const { return this == GetZero().get() || this == GetOne().get(); }

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
    std::pair<unsigned int, unsigned int> NumOfNodes() const;
    DirectedGraph GenerateDiagram() const;

    void GenerateDiagram(DirectedGraph &diagram, const DVertexDesc &parent) const;

    friend size_t hash_value(const MetaNode &node);

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

    inline size_t MemUsage() const {
        size_t apsize = sizeof(ANDNodePtr) + sizeof(size_t);
        size_t temp = sizeof(varID) + sizeof(card) +
                sizeof(children) +
                sizeof(hashVal) + (children.size() * apsize);
        BOOST_FOREACH(const ANDNodePtr &i, children) {
            temp += i->MemUsage();
        }
        return temp;
    }

};

typedef MetaNode::MetaNodePtr MetaNodePtr;
typedef MetaNode::ANDNodePtr ANDNodePtr;

bool operator==(const MetaNodePtr &lhs, const MetaNodePtr &rhs);
bool operator!=(const MetaNodePtr &lhs, const MetaNodePtr &rhs);
bool operator==(const ANDNodePtr &lhs, const ANDNodePtr &rhs);
bool operator!=(const ANDNodePtr &lhs, const ANDNodePtr &rhs);

} // end of aomdd namespace

#endif
