/*
 *  Graph.cpp
 *  aomdd
 *
 *  Created by William Lam on Jun 7, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#include "Graph.h"

namespace aomdd {
using namespace std;

Graph::Graph() : root(-1) {
}

Graph::Graph(int n, const vector<Scope> &scopes) :
    g(n), root(-1), inducedWidth(-1) {

    // For each function...
    for (unsigned int i = 0; i < scopes.size(); ++i) {
        // get the scope...
        list<int> scopeVars(scopes[i].GetOrdering());

        // Enumerate all pairs and add to graph
        while (!scopeVars.empty()) {
            int j = scopeVars.front();
            scopeVars.pop_front();
            list<int>::iterator it = scopeVars.begin();
            for (; it != scopeVars.end(); ++it) {
                add_edge(j, *it, g);
            }
        }
    }
}

const UndirectedGraph &Graph::GetGraph() const {
    return g;
}
pair<Vertex, Vertex> Graph::GetVertices() const {
    return vertices(g);
}

void Graph::InduceEdges(const list<int> &ordering) {
    this->ordering = ordering;
    root = ordering.front();
    inducedWidth = -1;
    boost::unordered_set<int> remainingNodes;
    list<int>::const_reverse_iterator rit = ordering.rbegin();
    for (; rit != ordering.rend(); ++rit) {
        remainingNodes.insert(*rit);
    }
    rit = ordering.rbegin();
    for (; rit != ordering.rend(); ++rit) {
        if (*rit == ordering.front()) break;
        remainingNodes.erase(*rit);
        Edge eout, eout_end;
        tie(eout, eout_end) = out_edges(*rit, g);
        // Generate parent list
        list<int> parents;
        for (; eout != eout_end; ++eout) {
            int outVertex = target(*eout, g);
            if (remainingNodes.find(outVertex) != remainingNodes.end()) {
                parents.push_back(outVertex);
            }
        }
        if ((int)parents.size() > inducedWidth) {
            inducedWidth = parents.size();
        }
        while (!parents.empty()) {
            int i = parents.front();
            parents.pop_front();
            list<int>::iterator pit = parents.begin();
            for (; pit != parents.end(); ++pit) {
                add_edge(i, *pit, g);
            }
        }
    }
}

int Graph::GetInducedWidth() const {
    assert(inducedWidth != -1);
    return inducedWidth;
}

list<int> Graph::GetOrdering() const {
    return ordering;
}

Graph::~Graph() {
}

} // end of aomdd namespace
