/*
 *  Graph.h
 *  aomdd
 *
 *  Created by William Lam on Jun 7, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

// Constructs the moral graph of a graphical model, given its scopes

#ifndef GRAPH_H_
#define GRAPH_H_

#include "base.h"
#include "graphbase.h"
#include "Scope.h"


namespace aomdd {


class Graph {
    UndirectedGraph g;
    std::list<int> ordering;
    int root;
    int inducedWidth;
    Graph();
public:
    Graph(int n, const std::vector<Scope> &scopes);
    virtual ~Graph();

    const UndirectedGraph &GetGraph() const;
    std::pair<Vertex, Vertex> GetVertices() const;

    void InduceEdges(const std::list<int> &ordering);

    int GetRoot() const { return root; }
    int GetInducedWidth() const;
    std::list<int> GetOrdering() const;
};

} // end of aomdd namespace

#endif /* GRAPH_H_ */
