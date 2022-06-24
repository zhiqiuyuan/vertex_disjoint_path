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
        for (VID_TYPE n : get_neighbors(i))
        {
            std::cout << n << " ";
        }
        std::cout << std::endl;
    }
}
void Graph::print_new2old_graph(const std::unordered_map<VID_TYPE, VID_TYPE> &new2old)
{
    print_with_colorln(BLUE, "\tmap new to old:");
    std::cout << "vertex_num:" << vertex_num << std::endl;
    std::cout << "vertices:";
    for (VID_TYPE i = 0; i < vertex_num; ++i)
    {
        std::cout << new2old.at(i) << " ";
    }
    std::cout << "\nneighbors:" << std::endl;
    for (VID_TYPE i = 0; i < vertex_num; ++i)
    {
        std::cout << new2old.at(i) << ": ";
        for (VID_TYPE n : get_neighbors(i))
        {
            std::cout << new2old.at(n) << " ";
        }
        std::cout << std::endl;
    }
}

void MemGraph::write_graph(VID_TYPE &s, VID_TYPE &t, std::unordered_map<VID_TYPE, VID_TYPE> &new2old, const std::vector<VID_TYPE> &V)
{
    std::unordered_map<VID_TYPE, VID_TYPE> old2new;
    std::set<VID_TYPE> old;
    VID_TYPE n = V.size();
    for (VID_TYPE i = 0; i < n; ++i)
    {
        old.insert(V[i]);
    }
    VID_TYPE i = 0;
    for (VID_TYPE oldv : old) // ensure mapping using seq in set
    {
        old2new[oldv] = i;
        new2old[i] = oldv;
        ++i;
    }

    std::vector<std::vector<VID_TYPE>> new_neighbors(n);
    std::vector<VID_TYPE> old_neighbor;
    VID_TYPE newv;
    for (VID_TYPE oldv : V)
    {
        newv = old2new[oldv];
        old_neighbor = neighbors[oldv];
        for (VID_TYPE v : old_neighbor)
        {
            if (old.count(v))
            {
                new_neighbors[newv].push_back(old2new[v]);
            }
        }
    }
    neighbors = new_neighbors;
    vertex_num = n;
    s = old2new[s];
    t = old2new[t];

#if DEBUG_LEVEL <= TRACE
    print_graph();
    print_new2old_graph(new2old);
    print_with_colorln(BLUE, "\told2new:");
    print_umapln(old2new);
#endif //#if DEBUG_LEVEL <= TRACE
}
void MemGraph::write_graph(VID_TYPE &s, VID_TYPE &t, std::unordered_map<VID_TYPE, VID_TYPE> &new2old, const std::set<VID_TYPE> &V)
{
    std::vector<VID_TYPE> vv;
    for (VID_TYPE v : V)
    {
        vv.push_back(v);
    }
    write_graph(s, t, new2old, vv);
}
