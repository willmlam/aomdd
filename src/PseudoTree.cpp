/*
 *  PseudoTree.cpp
 *  aomdd
 *
 *  Created by William Lam on Jun 21, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#include "PseudoTree.h"
#include <stack>
#include <set>

namespace aomdd {
using namespace std;

PseudoTree::PseudoTree() {
}

PseudoTree::PseudoTree(const Graph &inducedGraph)
: inducedWidth(inducedGraph.GetInducedWidth()), hasDummy(false) {
    DFSGenerator(inducedGraph);
}

int PseudoTree::GetNumberOfNodes() const {
    return num_vertices(g);
}

int PseudoTree::GetInducedWidth() const {
    assert(inducedWidth != -1);
    return inducedWidth;
}

unsigned int PseudoTree::GetHeight() const {
    if (num_vertices(g) <= 1) return 0;
    return GetHeight(root) - hasDummy;
}

unsigned int PseudoTree::GetHeight(int r) const {
    DEdge out, out_end;
    tie(out, out_end) = out_edges(r, g);
    if (out == out_end) {
        return 0;
    }
    int subtreeMax = -1;
    for (; out != out_end; ++out) {
       int subtreeHeight = GetHeight(target(*out, g));
       if (subtreeHeight > subtreeMax) {
           subtreeMax = subtreeHeight;
       }
    }
    return 1 + subtreeMax;
}

void PseudoTree::DFSGenerator(const Graph &inducedGraph) {
    using namespace boost;
    DFSTreeGenerator vis(g);
    UndirectedGraph ig(inducedGraph.GetGraph());
    vector<int> component(num_vertices(ig));
    root = inducedGraph.GetRoot();
    int numOfComponents = connected_components(ig, &component[0]);

    list<int> ordering = inducedGraph.GetOrdering();
    if (numOfComponents > 1) {
        hasDummy = true;
        // Make a dummy root
        root = component.size();
        // Connect components to dummy root
        vector<bool> connected(6, false);
        list<int>::const_iterator it = ordering.begin();
        for (; it != ordering.end(); ++it) {
            if (!connected[component[*it]]) {
                add_edge(root, *it, ig);
                connected[component[*it]] = true;
            }
        }
        ordering.push_front(root);
    }

    undirected_dfs(ig, ordering, root_vertex(VertexDesc(root)).
            visitor(vis).
            edge_color_map(get(edge_color,ig)));
}




// To finish later
void PseudoTree::BalancingGenerator(const Graph &inducedGraph) {
    // First, see if the graph has > 1 connected component
    UndirectedGraph ig(inducedGraph.GetGraph());
    int numVertices = num_vertices(ig);

    // For each node, find node to remove that best reduces the
    // max component size
    for (int i = 0; i < numVertices; ++i) {
        vector<int> component(num_vertices(ig));
//        int numComponents = connected_components(ig, &component[0]);
    }
}

pair<DirectedGraph, int> PseudoTree::GenerateEmbeddable(const Scope &s) const {
    using namespace boost;
    DirectedGraph embeddableTree;
    int embedRoot = -1;
    EmbedTreeGenerator vis(embeddableTree, s, embedRoot);
    depth_first_search(g, root_vertex(VertexDesc(root)).
            visitor(vis).
            edge_color_map(get(edge_color,g)));
    return make_pair<DirectedGraph, int>(embeddableTree, embedRoot);
}

PseudoTree::~PseudoTree() {
}

} // end of aomdd namespace

