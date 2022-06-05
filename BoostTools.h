#ifndef _BOOSTTOOLS_H
#define _BOOSTTOOLS_H
#include <boost/config.hpp>
#include <vector>
#include <list>
#include <boost/graph/biconnected_components.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <iterator>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "DataSetGraph.h"

namespace boost
{
    struct edge_component_t
    {
        enum
        {
            num = 555
        };
        typedef edge_property_tag kind;
    };
}
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property,
                              boost::property<boost::edge_component_t, std::size_t>>
    graph_t;
typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_t;

struct Edge
{
    int sv;
    int tv;
    Edge(int s, int t) : sv(s), tv(t) {}
    Edge() : sv(0), tv(0) {}
    bool operator<(const Edge &another)
    {
        if (sv != another.sv)
        {
            return sv < another.sv;
        }
        return tv < another.tv;
    }
    bool operator==(const Edge &another) const
    {
        return sv == another.sv && tv == another.tv;
    }
    std::string str()
    {
        return std::to_string(sv) + "_" + std::to_string(tv);
    }
};

void to_boost_graph(graph_t &g, Graph &src);
// return whether block(s)==block(t)
// if yes, return edges of block(s) in ascending order to file(for each edge: s<t) before and after mapping(both after unique)
// and return s t after mapping
bool st_biconnected_component(graph_t &g, int &s, int &t, std::vector<Edge> &before, std::vector<Edge> &after, bool print_comp_detail);

// correct, but stack overflow is possible for big graph like amazon6061
bool rec_st_biconnected_component(Graph &g, int &s, int &t, std::unordered_map<int, int> &new2old);

#endif //_BOOSTTOOLS_H