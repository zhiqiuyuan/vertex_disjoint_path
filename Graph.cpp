#include "Graph.h"

void Graph::generate_rand_vpairs(int pairs_cnt, std::set<std::pair<VID_TYPE, VID_TYPE>> &st_pairs)
{
    if (pairs_cnt > 0)
    {
        VID_TYPE n = vertex_num;
        VID_TYPE s, t;
        // size_t pre_sz = 0;
        while (st_pairs.size() < (size_t)pairs_cnt)
        {
            s = Rand(n); // NOTE: pass as long long, return long long
            t = Rand(n);
            while (t == s)
            {
                t = Rand(n);
            }
            if (s > t)
            {
                VID_TYPE tmp = s;
                s = t;
                t = tmp;
            }
            st_pairs.insert(std::pair<VID_TYPE, VID_TYPE>(s, t));
            /*
            if (st_pairs.size() != pre_sz)
            {
                std::cout << "\tst_pairs.size():" << st_pairs.size() << std::endl;
            }
            pre_sz = st_pairs.size();
            */
        }
    }
}
void Graph::print_graph()
{
    std::cout << "vertex_num:" << vertex_num << std::endl;
    std::cout << "neighbors:" << std::endl;
    for (VID_TYPE i = 0; i < vertex_num; ++i)
    {
        std::cout << i << ": ";
        std::vector<VID_TYPE> nbrs = get_neighbors(i);
        for (VID_TYPE n : nbrs)
        {
            std::cout << n << " ";
        }
        std::cout << std::endl;
    }
}
void MemGraph::write_graph(const std::unordered_set<VID_TYPE> &V)
{
    vertex_num = V.size();

    VID_TYPE sz = neighbors.size();
    for (VID_TYPE v = 0; v < sz; ++v)
    {
        if (V.count(v) == 0)
        {
            neighbors[v].clear();
            neighbors[v].shrink_to_fit(); // release rest memory
        }
        else
        {
            VID_TYPE idx = 0;
            for (VID_TYPE w : neighbors[v])
            {
                if (V.count(w))
                {
                    neighbors[v][idx++] = w;
                }
            }
            neighbors[v].resize(idx);
            neighbors[v].shrink_to_fit();
        }
    }

#if DEBUG_LEVEL <= TRACE
    print_graph();
#endif //#if DEBUG_LEVEL <= TRACE
}
void MemGraph::write_graph(const std::vector<VID_TYPE> &Vvec)
{
    std::unordered_set<VID_TYPE> V;
    for (VID_TYPE v : Vvec)
    {
        V.insert(v);
    }
    write_graph(V);
}
