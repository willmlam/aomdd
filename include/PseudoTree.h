/*
 *  PseudoTree.h
 *  aomdd
 *
 *  Created by William Lam on Apr 8, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#ifndef PSEUDOTREE_H_AOMDD
#define PSEUDOTREE_H_AOMDD

#include "graphbase.h"
#include "./utils.h"
#include "./Graph.h"
#include "Scope.h"
#include "Model.h"

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

    int numScopeUnfinished;

    // Unfinished vertices that are in the scope
    std::stack<int> unfinished;
    std::stack<int> candidateConnectors;
    std::vector<int> disconnectedRoots;

    EmbedTreeGenerator(DirectedGraph &treeIn, const Scope &s, int &rootIn)
    : treeRef(treeIn), sRef(s), root(rootIn) {
        numScopeUnfinished = sRef.GetNumVars();
    }

    template <class Edge, class Graph>
    void tree_edge(Edge e, const Graph &g) {
        int targetNode = target(e, g);
        if ( root != -1 && sRef.VarExists(targetNode) ) {
            assert(!unfinished.empty() || !disconnectedRoots.empty() );
            if (!unfinished.empty()) {
                add_edge(unfinished.top(), targetNode, treeRef);
            }
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
        if ( sRef.VarExists(v) && unfinished.top() == int(v)) {
            numScopeUnfinished--;
            add_edge(v,v,treeRef);
            remove_edge(v,v,treeRef);
            if (in_degree(int(v), treeRef) == 0) {
                disconnectedRoots.push_back(v);
                DInEdge ei, ei_end;
                tie(ei, ei_end) = in_edges(v, g);
                int candidate = source(*ei, g);
                if (candidateConnectors.empty() || candidateConnectors.top() != candidate) {
                    candidateConnectors.push(candidate);
                }
            }
            unfinished.pop();
        }
        else if ( !candidateConnectors.empty() && candidateConnectors.top() == int(v) ) {
            candidateConnectors.pop();
            if (candidateConnectors.size() == 0 && numScopeUnfinished == 0) {
                for (unsigned int i = 0; i < disconnectedRoots.size(); ++i) {
                    add_edge(v, disconnectedRoots[i], treeRef);
                }
                /*
                BOOST_FOREACH(int dv, disconnectedRoots) {
                    add_edge(v, dv, treeRef);
                }
                */
                root = v;
            }
            else {
                DInEdge ei, ei_end;
                tie(ei, ei_end) = in_edges(v, g);
                int candidate = source(*ei, g);
                if (candidateConnectors.empty() || candidateConnectors.top() != candidate) {
                    candidateConnectors.push(candidate);
                }
            }
        }
    }
};

struct DescendantGenerator : public boost::dfs_visitor<> {
    std::vector< std::set<int> > &descendants;

    DescendantGenerator(std::vector< std::set<int> > &dIn) :
        descendants(dIn) {
    }

    template <class Graph, class Vertex>
    void finish_vertex(Vertex v, const Graph &g) {
        descendants[v].insert(v);
        DInEdge ei, ei_end;
        tie(ei, ei_end) = in_edges(v, g);
        for (; ei != ei_end; ++ei) {
            int parent = source(*ei, g);
            std::set<int> unionSet;
            BOOST_FOREACH(int d, descendants[v]) {
	            descendants[parent].insert(d);
            }
        }
    }
};

class PseudoTree {
    DirectedGraph g;
    int root;
    int inducedWidth;
    Scope s;
    bool hasDummy;
    std::vector<std::set<int> > context;
public:
    PseudoTree();

    PseudoTree(const Model &m);
    // Assumes graph is an induced graph...
    PseudoTree(const Graph &inducedGraph, const Scope &sIn);
    virtual ~PseudoTree();

    const DirectedGraph &GetTree() const { return g; }
    int GetNumberOfNodes() const;
    int GetInducedWidth() const;
    unsigned int GetHeight() const;
    int GetRoot() const { return root; }
    bool HasDummy() const { return hasDummy; }
    inline const Scope &GetScope() const { return s; }
    inline const std::vector<std::set<int> > &GetContexts() { return context; }

    // Generates an embeddable pseudo tree to use as a backbone tree
    std::pair<DirectedGraph, int> GenerateEmbeddable(const Scope &s) const;

private:
    void DFSGenerator(const Graph &inducedGraph);
    void BalancingGenerator(const Graph &inducedGraph);

    void ComputeContext(const Graph &inducedGraph);
    const std::set<int> &ComputeContext(int r, const Graph &inducedGraph, std::set<int> &ancestors);
    unsigned int GetHeight(int r) const;
};

} // end of aomdd namespace

#endif /* PSEUDOTREE_H_ */
