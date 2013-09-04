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
typedef graph_traits<UndirectedGraph>::edge_descriptor EdgeDesc;
typedef graph_traits<UndirectedGraph>::vertex_iterator Vertex;
typedef graph_traits<UndirectedGraph>::out_edge_iterator Edge;

typedef adjacency_list<setS, vecS, bidirectionalS, property<vertex_name_t, std::string> > DirectedGraph;
typedef graph_traits<DirectedGraph>::vertex_iterator DVertex;
typedef graph_traits<DirectedGraph>::vertex_descriptor DVertexDesc;
typedef graph_traits<DirectedGraph>::out_edge_iterator DEdge;
typedef graph_traits<DirectedGraph>::in_edge_iterator DInEdge;

template <typename T>
inline std::string toString (const T &t) {
    std::stringstream ss;
    ss << t;
    return ss.str();
}

struct AOVertexProp {
    std::string label;
    NodeType type;

    AOVertexProp() {}
    AOVertexProp(std::string _label, NodeType _type) : label(_label), type(_type) {}
    AOVertexProp(int _label, NodeType _type) : label(toString(_label)), type(_type) {}
};

struct AOEdgeProp {
    std::string label;
    EdgeType type;

    AOEdgeProp() {}
    AOEdgeProp(std::string _label, EdgeType _type) : label(_label), type(_type) {}
    AOEdgeProp(double _label, EdgeType _type) : label(toString(_label)), type(_type) {}
};

typedef adjacency_list<setS, vecS, bidirectionalS, AOVertexProp, AOEdgeProp> AOGraph;

struct AOVertexPropWriter {
    public:
        AOVertexPropWriter(AOGraph _g) : g(_g) {}

        template <class Vertex>
        void operator()(std::ostream &out, const Vertex& v) const {
            std::string label = g[v].label;
            bool isTerminal = false;
            if (label == "-2") {
                label = "0";
                isTerminal = true;
            }
            else if (label == "-1") {
                label = "1";
                isTerminal = true;
            }
            out << "[label=\"" << label << "\"";
            if (isTerminal) {
                out << ",shape=box";
            }
            else if (g[v].type == OR) {
                out << ",shape=circle";
            }
            else if (g[v].type == AND) {
                out << ",shape=box,width=0.25,height=0.25,fixedsize=true";
            }
            out << "]" << std::endl;
        }

    private:
        AOGraph g;
};

struct AOEdgePropWriter {
    public:
        AOEdgePropWriter(AOGraph _g) : g(_g) {}

        template <class Edge>
        void operator()(std::ostream &out, const Edge& e) const {
            out << "[";
            if (g[e].label != "1" || g[e].type == ORtoAND)
                out << "label=" << g[e].label << ",";
            out << "minlen=1,";
            out << "splines=line";
            out << "]" << std::endl;
        }

    private:
        AOGraph g;
};


} // end of aomdd namespace

#endif /* GRAPHBASE_H_ */
