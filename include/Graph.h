/*
 *  Graph.h
 *  aomdd
 *
 *  Created by William Lam on Jun 7, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

// Constructs the moral graph of a graphical model, given its scopes

#ifndef GRAPH_H_AOMDD
#define GRAPH_H_AOMDD

#include "base.h"
#include "graphbase.h"
#include "Scope.h"
#include <functional>


namespace aomdd {

struct MinFill : public std::binary_function<UndirectedGraph, int, int> {
    virtual int operator() (UndirectedGraph g, int v) {
        UndirectedGraph gCopy(g);
        std::list<int> parents;
        Edge ei, ei_end;
        tie(ei, ei_end) = out_edges(v, gCopy);
        for(; ei != ei_end; ++ei) {
            parents.push_back(int(target(*ei, g)));
        }
        clear_vertex(v, gCopy);
        remove_vertex(v, gCopy);

        EdgeDesc ed;
        bool isNewEdge;

        int cost = 0;

        while (!parents.empty()) {
            int i = parents.front();
            parents.pop_front();
            std::list<int>::iterator pit = parents.begin();
            for (; pit != parents.end(); ++pit) {
                tie(ed, isNewEdge) = add_edge(i, *pit, gCopy);
                if (isNewEdge) cost++;
            }
        }
        return cost;
    }
};


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

    template <typename CostFunction>
    std::list<int> ComputeOrdering(CostFunction &cf) {
	    std::list<int> cOrder;
	    UndirectedGraph gCopy(g);
	    std::list<int> remainingNodes;
	    int n = num_vertices(gCopy);
	    for (int i = 0; i < n; ++i) {
	        remainingNodes.push_back(i);
	    }

	    while(!remainingNodes.empty()) {
		    std::vector<int> zeroCost;
	        int minCost = n;
	        int elimVar = -1;
	        BOOST_FOREACH(int v, remainingNodes) {
	            int cost = cf(gCopy, v);
	            if (cost == 0) zeroCost.push_back(v);
	            if (cost < minCost && cost > 0) {
	                minCost = cost;
	                elimVar = v;
	            }
	        }
	        // Remove 0 cost nodes
	        BOOST_FOREACH(int v, zeroCost) {
	            remainingNodes.remove(v);
	            clear_vertex(v, gCopy);
		        cOrder.push_front(v);
	        }
	        std::cout << "Found minCost=" << minCost << " with var " << elimVar << std::endl;
	        remainingNodes.remove(elimVar);
	        cOrder.push_front(elimVar);
	        Edge e, e_end;
	        tie(e, e_end) = out_edges(elimVar, gCopy);
	        // Generate parent list
	        std::list<int> parents;
	        for (; e != e_end; ++e) {
	            int outVertex = target(*e, gCopy);
	            if (std::find(remainingNodes.begin(), remainingNodes.end(), outVertex) !=
	                    remainingNodes.end()) {
	                parents.push_back(outVertex);
	            }
	        }
	        BOOST_FOREACH(int ver, parents) {
	            std::cout << ver << std::endl;
	        }

	        clear_vertex(elimVar, gCopy);

	        while (!parents.empty()) {
	            int i = parents.front();
	            parents.pop_front();
	            std::list<int>::iterator pit = parents.begin();
	            for (; pit != parents.end(); ++pit) {
			        EdgeDesc ed;
			        bool isNewEdge;
	                tie(ed,isNewEdge) = add_edge(i, *pit, gCopy);
	                std::cout << "Adding edge" << " (" << i << "," << *pit << ")" << std::endl;
	                std::cout << "is " << (isNewEdge ? "new" : "old") << std::endl;
	            }
	        }
	    }
	    return cOrder;
    }

    int GetRoot() const { return root; }
    int GetInducedWidth() const;
    std::list<int> GetOrdering() const;
};

} // end of aomdd namespace

#endif /* GRAPH_H_ */
