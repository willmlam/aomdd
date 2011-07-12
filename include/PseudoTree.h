/*
 *  PseudoTree.h
 *  aomdd
 *
 *  Created by William Lam on Apr 8, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#ifndef PSEUDOTREE_H_
#define PSEUDOTREE_H_

#include "graphbase.h"
#include "utils.h"
#include "Graph.h"
#include "Scope.h"

// Should take in an induced graph

namespace aomdd {

struct DFSTreeGenerator : public boost::dfs_visitor<> {
    DirectedGraph &treeRef;

    DFSTreeGenerator(DirectedGraph &treeIn) : treeRef(treeIn) {
    }

    template <class Edge, class Graph>
    void tree_edge(Edge e, const Graph &g) {
        add_edge(source(e, g), target(e, g), treeRef);
    }
};

struct EmbedTreeGenerator : public boost::dfs_visitor<> {
    DirectedGraph &treeRef;
    const Scope &sRef;
    int &root;
    // Unfinished vertices that are in the scope
    std::stack<int> unfinished;

    EmbedTreeGenerator(DirectedGraph &treeIn, const Scope &s, int &rootIn)
    : treeRef(treeIn), sRef(s), root(rootIn) {
    }

    template <class Edge, class Graph>
    void tree_edge(Edge e, const Graph &g) {
        int targetNode = target(e, g);
        if ( root != -1 && sRef.VarExists(targetNode) ) {
            assert(!unfinished.empty());
            add_edge(unfinished.top(), targetNode, treeRef);
        }

    }

    template <class Vertex, class Graph>
    void discover_vertex(Vertex v, const Graph &g) {
        if ( sRef.VarExists(v) ) {
            if (root == -1) root = v;
            unfinished.push(v);
        }
    }

    template <class Vertex, class Graph>
    void finish_vertex(Vertex v, const Graph &g) {
        if ( sRef.VarExists(v) && unfinished.top() == int(v))
            unfinished.pop();
    }
};

class PseudoTree {
    DirectedGraph g;
    int root;
    int inducedWidth;
    bool hasDummy;
public:
    PseudoTree();
    // Assumes graph is an induced graph...
    PseudoTree(const Graph &inducedGraph);
    virtual ~PseudoTree();

    const DirectedGraph &GetTree() const { return g; }
    int GetNumberOfNodes() const;
    int GetInducedWidth() const;
    unsigned int GetHeight() const;
    int GetRoot() const { return root; }
    bool HasDummy() const { return hasDummy; }

    // Generates an embeddable pseudo tree to use as a backbone tree
    std::pair<DirectedGraph, int> GenerateEmbeddable(const Scope &s) const;

private:
    void DFSGenerator(const Graph &inducedGraph);
    void BalancingGenerator(const Graph &inducedGraph);
    unsigned int GetHeight(int r) const;
};

} // end of aomdd namespace

#endif /* PSEUDOTREE_H_ */
