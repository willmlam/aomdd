/*
 *  graphbase.h
 *  aomdd
 *
 *  Created by William Lam on Jun 21, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#ifndef GRAPHBASE_H_
#define GRAPHBASE_H_

#include <boost/graph/adjacency_list.hpp>
//#include <boost/graph/undirected_dfs.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/reverse_graph.hpp>
#include "base.h"

// Using the customized version that takes in an ordering
#include "undirected_dfs.hpp"

namespace aomdd {
using namespace boost;

typedef adjacency_list<setS, vecS, undirectedS, no_property, property<
        edge_color_t, default_color_type> > UndirectedGraph;
typedef graph_traits<UndirectedGraph>::vertex_descriptor VertexDesc;
typedef graph_traits<UndirectedGraph>::vertex_iterator Vertex;
typedef graph_traits<UndirectedGraph>::out_edge_iterator Edge;

typedef adjacency_list<setS, vecS, bidirectionalS, property<vertex_name_t, std::string> > DirectedGraph;
typedef graph_traits<DirectedGraph>::vertex_iterator DVertex;
typedef graph_traits<DirectedGraph>::vertex_descriptor DVertexDesc;
typedef graph_traits<DirectedGraph>::out_edge_iterator DEdge;
typedef graph_traits<DirectedGraph>::in_edge_iterator DInEdge;



} // end of aomdd namespace

#endif /* GRAPHBASE_H_ */
