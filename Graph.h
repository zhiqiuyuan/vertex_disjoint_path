#ifndef _GRAPH_H
#define _GRAPH_H

#include "tools.h"
// v in file are vid
// vid are [0,vertex_num)
// no parallel edge
class Graph
{
protected:
    VID_TYPE vertex_num;

public:
    Graph() : vertex_num(0) {}
    VID_TYPE vertexnum() { return vertex_num; }
    // return new vertex_num
    virtual VID_TYPE add_vertex() = 0;
    virtual void add_edge(VID_TYPE s, VID_TYPE t) = 0;
    virtual std::vector<VID_TYPE> &get_neighbors(VID_TYPE vid) = 0;
    virtual void set_neighbors(VID_TYPE vid, std::vector<VID_TYPE> new_ns) = 0;
    virtual void set_all_neighbors(std::vector<std::vector<VID_TYPE>> &new_neighbors, VID_TYPE n) = 0;
    // return neighbors[vid]
    // NOTE: won't update vertex_num!! since all vertex id are not changed
    // NOTE: adjacent list for undirected graph: 2 entry 1 link
    virtual std::vector<VID_TYPE> delete_vertex(VID_TYPE vid) = 0;
    virtual void recover_vertex(VID_TYPE vid, const std::vector<VID_TYPE> &nbrs) = 0;

    VID_TYPE get_degree(VID_TYPE vid)
    {
        assert(vid >= 0 && vid < vertex_num);
        return get_neighbors(vid).size();
    }
    std::vector<VID_TYPE> intersect_neighbors(VID_TYPE s, VID_TYPE t)
    {
        assert(s >= 0 && s < vertex_num && t >= 0 && t < vertex_num);
        std::vector<VID_TYPE> nbrs1 = get_neighbors(s), nbrs2 = get_neighbors(t);
        std::sort(nbrs1.begin(), nbrs1.end());
        std::sort(nbrs2.begin(), nbrs2.end());
        std::vector<VID_TYPE> re;
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
    void print_new2old_graph(const std::unordered_map<VID_TYPE, VID_TYPE> &new2old);
    void generate_rand_vpairs(int pairs_cnt, std::set<std::pair<VID_TYPE, VID_TYPE>> &st_pairs);

    // new g: vertex in V, all edges in g between V, remapping V->[0,|V|)
    // return new2old mapping, return new st and new g
    // new2old for translating path to the graph before reduction
    virtual void write_graph(VID_TYPE &s, VID_TYPE &t, std::unordered_map<VID_TYPE, VID_TYPE> &new2old, const std::vector<VID_TYPE> &V) = 0;
    virtual void write_graph(VID_TYPE &s, VID_TYPE &t, std::unordered_map<VID_TYPE, VID_TYPE> &new2old, const std::set<VID_TYPE> &V) = 0;

    // return build succeed or not
    virtual bool buildGraph(std::string fname) = 0;
};

class MemGraph : public Graph
{
protected:
    std::vector<std::vector<VID_TYPE>> neighbors;

public:
    MemGraph() {}
    VID_TYPE add_vertex()
    {
        ++vertex_num;
        neighbors.push_back({});
        return vertex_num;
    }
    void add_edge(VID_TYPE s, VID_TYPE t)
    {
        assert(s >= 0 && t >= 0);
        if (s >= vertex_num || t >= vertex_num)
        {
            vertex_num = std::max(s, t) + 1;
            neighbors.resize(vertex_num, {});
        }
        neighbors[s].push_back(t);
        neighbors[t].push_back(s);
    }
    VID_TYPE get_degree(VID_TYPE vid)
    {
        assert(vid >= 0 && vid < vertex_num);
        return neighbors[vid].size();
    }
    std::vector<VID_TYPE> &get_neighbors(VID_TYPE vid)
    {
        assert(vid >= 0 && vid < vertex_num);
        return neighbors[vid];
    }
    void set_neighbors(VID_TYPE vid, std::vector<VID_TYPE> new_ns)
    {
        assert(vid >= 0 && vid < vertex_num);
        neighbors[vid] = new_ns;
    }
    void set_all_neighbors(std::vector<std::vector<VID_TYPE>> &new_neighbors, VID_TYPE n)
    {
        neighbors = new_neighbors;
        vertex_num = n;
    }
    std::vector<VID_TYPE> delete_vertex(VID_TYPE vid)
    {
        assert(vid >= 0 && vid < vertex_num);
        for (VID_TYPE i = 0; i < vertex_num; ++i)
        {
            std::vector<VID_TYPE> &nbrs = neighbors[i];
            std::vector<VID_TYPE>::iterator it;
            if ((it = std::find(nbrs.begin(), nbrs.end(), vid)) != nbrs.end())
            {
                nbrs.erase(it);
            }
        }
        std::vector<VID_TYPE> tmp = neighbors[vid];
        neighbors[vid].clear();
        return tmp;
    }
    void recover_vertex(VID_TYPE vid, const std::vector<VID_TYPE> &nbrs)
    {
        assert(vid >= 0 && vid < vertex_num);
        neighbors[vid] = nbrs;
        for (VID_TYPE v : nbrs)
        {
            neighbors[v].push_back(vid);
        }
    }

    // create new adjancent list representation in std::vector<std::vector<VID_TYPE>> new_neighbors then copy
    void write_graph(VID_TYPE &s, VID_TYPE &t, std::unordered_map<VID_TYPE, VID_TYPE> &new2old, const std::vector<VID_TYPE> &V);
    void write_graph(VID_TYPE &s, VID_TYPE &t, std::unordered_map<VID_TYPE, VID_TYPE> &new2old, const std::set<VID_TYPE> &V);

    // load total graph into std::vector<std::vector<VID_TYPE>> neighbors
    virtual bool buildGraph(std::string fname) = 0;
};

/*
class DBGraph : public Graph
{
protected:
    DbEngine *db;

public:
    DBGraph() {}
};
*/

#endif //_GRAPH_H