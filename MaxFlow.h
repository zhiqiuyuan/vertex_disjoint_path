#ifndef _MAXFLOW_H
#define _MAXFLOW_H

#include "tools.h"
#include "Graph.h"

// undirected graph -(directed+split vertex)-> SplitedGraph
// vid[0,ori_vertex_num) -> [0,2*ori_vertex_num)
class SplitedGraph
{
    VID_TYPE ori_vertex_num;
    std::vector<std::vector<VID_TYPE>> head_out_neighbors; // vid<ori_vertex_num
                                                           // each entry has exactly one neighbor
                                                           // vector<vector> for get_out_neighbors's "const std::vector<VID_TYPE> &" return
                                                           // for each vid: init: vid+ori_vertex_num
    std::vector<std::vector<VID_TYPE>> tail_out_neighbors; // vid>=ori_vertex_num
                                                           // for each vid: init: neighbors[vid], remove old_s
    VID_TYPE s, t;                                         // new s t in SplitedGraph

private:
    std::vector<VID_TYPE> &get_mutable_out_neighbors(VID_TYPE vid)
    {
        assert(vid >= 0 && vid < (ori_vertex_num << 1)); //*2
        if (vid < ori_vertex_num)
        {
            return head_out_neighbors[vid];
        }
        else
        {
            return tail_out_neighbors[vid - ori_vertex_num];
        }
    }

public:
    // tail_out_neighbors: move construct
    // head_out_neighbors: alloc first(避免vector从0开始一个个增长的翻倍重新分配和复制)
    SplitedGraph(MemGraph &&g, VID_TYPE ori_s, VID_TYPE ori_t) : ori_vertex_num(g.vertex_num),
                                                                 head_out_neighbors(ori_vertex_num),
                                                                 tail_out_neighbors(std::move(g.neighbors))
    {
        s = ori_s + ori_vertex_num;
        t = ori_t;
        for (VID_TYPE v = 0; v < ori_vertex_num; ++v)
        {
            head_out_neighbors[v].push_back(v + ori_vertex_num);
        }
        // remove ori_s from tail_out_neighbors
        // all ori_s's neighbors has an entry for ori_s
        for (VID_TYPE ori_s_nbr : tail_out_neighbors[ori_s])
        {
            auto it = std::find(tail_out_neighbors[ori_s_nbr].begin(), tail_out_neighbors[ori_s_nbr].end(), ori_s);
            tail_out_neighbors[ori_s_nbr].erase(it);
        }
    }
    ~SplitedGraph() {}
#if DEBUG_LEVEL <= TRACE
    void print_out_neighbors(VID_TYPE v)
    {
        std::cout << v << ": ";
        for (VID_TYPE u : get_out_neighbors(v))
        {
            std::cout << u << " ";
        }
        std::cout << std::endl;
    }
    void print_graph()
    {
        for (VID_TYPE v = 0; v < ori_vertex_num; ++v)
        {
            print_out_neighbors(v);
            print_out_neighbors(v + ori_vertex_num);
        }
    }
    // format for https://csacademy.com/app/graph_editor/ to visualize
    void print_graph_plot_format()
    {
        VID_TYPE n = (ori_vertex_num << 1);
        for (VID_TYPE v = 0; v < n; ++v)
        {
            for (VID_TYPE u : get_out_neighbors(v))
            {
                std::cout << v << " " << u << std::endl;
            }
        }
    }
#endif //#if DEBUG_LEVEL <= TRACE
    VID_TYPE get_s()
    {
        return s;
    }
    VID_TYPE get_t() { return t; }
    VID_TYPE get_out_degree(VID_TYPE vid) const
    {
        return get_out_neighbors(vid).size();
    }
    const std::vector<VID_TYPE> &get_out_neighbors(VID_TYPE vid) const
    {
        // call private get_out_neighbors func then cast neighbors to const
        return static_cast<const std::vector<VID_TYPE> &>(const_cast<SplitedGraph *>(this)->get_mutable_out_neighbors(vid));
    }
    // replace B's out_neighbor C with A (if C doesn's exist in B's out_neighbor, do nothing)
    // A=-1 only when B==s, C=-1 only when B==t
    void swap_neighbors(VID_TYPE B, VID_TYPE C, VID_TYPE A)
    {
        std::vector<VID_TYPE> &B_nbrs = get_mutable_out_neighbors(B);
        if (C != -1)
        {
            auto C_it = std::find(B_nbrs.begin(), B_nbrs.end(), C);
            if (C_it == B_nbrs.end())
            {
                return;
            }

            // B is s: remove C from B's out_neighbors
            if (A == -1)
            {
                B_nbrs.erase(C_it);
            }
            // replace B's out_neighbor C with A
            else
            {
                *C_it = A;
            }
        }
        // B is t: add A to B's out_neighbor
        else
        {
            B_nbrs.push_back(A);
        }
    }

    // dinic: label stage, fill vid2label
    // return whether labeling can reach t
    // return -1(TIME_EXCEED_RESULT) if time exceed
    // 考虑到label stage在标记到t以及同层孩子就结束，所有有标记的顶点应该不会很多，采用哈希结构存储顶点到label，而不是每个顶点在邻接表中存储label
    int dinic_label(std::unordered_map<VID_TYPE, VID_TYPE> &vid2label);

    // trace one path
    // return whether successfully trace a path, if so, fill it in path
    // return -1(TIME_EXCEED_RESULT) if time exceed
    int dinic_trace_path(const std::unordered_map<VID_TYPE, VID_TYPE> &vid2label, std::vector<VID_TYPE> &path);

    // reverse along path
    void reverse_path(const std::vector<VID_TYPE> &path);

    // first construct a new graph: edges are all edges in pre_disjoint_paths (all are edge disjoint), after removing dup edges
    // then all paths from s to t in this new graph is exactly the modified paths
    // then translate into original graph
    // disjoint_paths pass as empty
    // return whether successfully modify and translate all paths(fail eg: not edge disjoint), if so, fill disjoint_paths
    // return -1(TIME_EXCEED_RESULT) if time exceed
    int dinic_modify_path(const std::vector<std::vector<VID_TYPE>> &pre_disjoint_paths, std::vector<std::vector<VID_TYPE>> &disjoint_paths);

    // translate modified_path to path
    // return whether successfully translate modified_path(fail eg: modified_path in wrong vertex sequence format), if so, fill path
    // when constructing std::vector<VID_TYPE> modified_path, using move constructor(by passing std::move() to modified_path)
    int translate_path(std::vector<VID_TYPE> modified_path, std::vector<VID_TYPE> &path);
};

// RAII
class SplitedGraphWrapper
{
public:
    SplitedGraph *G;
    SplitedGraphWrapper(MemGraph &&g, VID_TYPE s, VID_TYPE t)
    {
        G = new SplitedGraph(std::move(g), s, t);
    }
    ~SplitedGraphWrapper()
    {
        delete G;
    }
};

// run alg and print result
int maxflow(MemGraph &g, VID_TYPE s, VID_TYPE t);

// return 2VDPP has solution or not, if yes return solution in path1 and path2
// return -1(TIME_EXCEED_RESULT) if time exceed
int maxflow_based(MemGraph &g, VID_TYPE s, VID_TYPE t, VID_TYPE disjoint_path_num, std::vector<std::vector<VID_TYPE>> &disjoint_paths);

#endif //_MAXFLOW_H