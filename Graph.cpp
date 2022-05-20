#include "Graph.h"

void Graph::generate_rand_vpairs(int pairs_cnt, std::set<std::pair<int, int>> &st_pairs)
{
    int n = neighbors.size();
    int s, t;
    while (st_pairs.size() < pairs_cnt)
    {
        s = Rand(n);
        t = Rand(n);
        while (t == s)
        {
            t = Rand(n);
        }
        if (s > t)
        {
            int tmp = s;
            s = t;
            t = tmp;
        }
        st_pairs.insert(std::pair<int, int>(s, t));
    }
}

void write_graph(Graph &g, int &s, int &t, std::unordered_map<int, int> &new2old, const std::vector<int> &V)
{
    std::unordered_map<int, int> old2new;
    std::set<int> old;
    int n = V.size();
    for (int i = 0; i < n; ++i)
    {
        old.insert(V[i]);
    }
    int i = 0;
    for (int oldv : old) // ensure mapping using seq in set
    {
        old2new[oldv] = i;
        new2old[i] = oldv;
        ++i;
    }
    std::vector<std::vector<int>> new_neighbors(n);
    // std::vector<std::vector<int>> new_neighbors(g.vertex_num); // WARN!!NO MAPPING NOW!
    std::vector<int> old_neighbor;
    int newv;
    for (int oldv : V)
    {
        newv = old2new[oldv];
        old_neighbor = g.get_neighbors(oldv);
        for (int v : old_neighbor)
        {
            if (old.count(v))
            {
                new_neighbors[newv].push_back(old2new[v]);
                // new_neighbors[oldv].push_back(v); // WARN!!NO MAPPING NOW!
            }
        }
    }
    g.neighbors = new_neighbors;
    g.vertex_num = n; // if commented:WARN!!NO MAPPING NOW!
    s = old2new[s];
    t = old2new[t];
}
void write_graph(Graph &g, int &s, int &t, std::unordered_map<int, int> &new2old, const std::set<int> &V)
{
    std::vector<int> vv;
    for (int v : V)
    {
        vv.push_back(v);
    }
    write_graph(g, s, t, new2old, vv);
}
