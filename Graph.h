#ifndef _GRAPH_H
#define _GRAPH_H

#include "tools.h"

// v in file are vid
// vid are [0,vertex_num)
class Graph
{
protected:
    std::vector<std::vector<int>> neighbors;
    int vertex_num;

public:
    int vertexnum() { return vertex_num; }
    int get_degree(int vid)
    {
        assert(vid >= 0 && vid < vertex_num);
        return neighbors[vid].size();
    }
    std::vector<int> &get_neighbors(int vid)
    {
        assert(vid >= 0 && vid < vertex_num);
        return neighbors[vid];
    }

    void generate_rand_vpairs(int pairs_cnt, std::set<std::pair<int, int>> &st_pairs);

    //全图一次性全部加载到内存 // fill Graph member
    // return build succeed or not
    virtual bool buildGraph(std::string fname) = 0;

    friend void write_graph(Graph &g, int &s, int &t, std::vector<int> &V);
};

// new g: vertex in V, all edges in g between V, remapping V->[0,|V|)(mapping st as well)
void write_graph(Graph &g, int &s, int &t, std::vector<int> &V);
// do bctree decomposing from s, stop when finishing block(s)
// return whether block(s)==block(t), and if so, write block(s) to g
bool st_biconnected_component(Graph &g, int &s, int &t);

#endif //_GRAPH_H