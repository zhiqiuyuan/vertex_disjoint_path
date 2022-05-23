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
    std::vector<int> intersect_neighbors(int s, int t)
    {
        assert(s >= 0 && s < vertex_num && t >= 0 && t < vertex_num);
        std::vector<int> nbrs1 = neighbors[s], nbrs2 = neighbors[t];
        std::sort(nbrs1.begin(), nbrs1.end());
        std::sort(nbrs2.begin(), nbrs2.end());
        std::vector<int> re;
        int i1 = 0, i2 = 0;
        int e1 = nbrs1.size(), e2 = nbrs2.size();
        while (i1 < e1 && i2 < e2)
        {
            if (nbrs1[i1] == nbrs2[i2])
            {
                re.push_back(nbrs1[i1]);
                i1++;
                i2++;
            }
            else if (nbrs1[i1] < nbrs2[i2])
            {
                i1++;
            }
            else
            {
                i2++;
            }
        }
        return re;
    }
    void print_graph();
    void print_new2old_graph(const std::unordered_map<int, int> &new2old);
    void generate_rand_vpairs(int pairs_cnt, std::set<std::pair<int, int>> &st_pairs);

    // load total graph into std::vector<std::vector<int>> neighbors
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