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
    void set_neighbors(int vid, std::vector<int> new_ns)
    {
        assert(vid >= 0 && vid < vertex_num);
        neighbors[vid] = new_ns;
    }
    // return neighbors[vid] and another side entry
    // NOTE: won't modify vertex_num!! since all vertex id are not changed
    // NOTE: adjacent list for undirected graph: 2 entry 1 link
    std::vector<int> delete_vertex(int vid)
    {
        assert(vid >= 0 && vid < vertex_num);
        for (int i = 0; i < vertex_num; ++i)
        {
            std::vector<int> &nbrs = neighbors[i];
            std::vector<int>::iterator it;
            if ((it = std::find(nbrs.begin(), nbrs.end(), vid)) != nbrs.end())
            {
                nbrs.erase(it);
            }
        }
        std::vector<int> tmp = neighbors[vid];
        neighbors[vid].clear();
        return tmp;
    }
    void recover_vertex(int vid, const std::vector<int> &nbrs)
    {
        assert(vid >= 0 && vid < vertex_num);
        neighbors[vid] = nbrs;
        for (int v : nbrs)
        {
            neighbors[v].push_back(vid);
        }
    }
    void generate_rand_vpairs(int pairs_cnt, std::set<std::pair<int, int>> &st_pairs);

    //全图一次性全部加载到内存 // fill Graph member
    // return build succeed or not
    virtual bool buildGraph(std::string fname) = 0;

    friend void write_graph(Graph &g, int &s, int &t, std::unordered_map<int, int> &new2old, const std::vector<int> &V);
};

// new g: vertex in V, all edges in g between V, remapping V->[0,|V|)
// return new2old mapping, return new st and new g
// new2old for translating path to the graph before reduction
void write_graph(Graph &g, int &s, int &t, std::unordered_map<int, int> &new2old, const std::vector<int> &V);
void write_graph(Graph &g, int &s, int &t, std::unordered_map<int, int> &new2old, const std::set<int> &V);

#endif //_GRAPH_H