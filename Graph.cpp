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

void write_graph(Graph &g, int &s, int &t, std::vector<int> &V)
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
        old2new[oldv] = i++;
    }
    // std::vector<std::vector<int>> new_neighbors(n);
    std::vector<std::vector<int>> new_neighbors(g.vertex_num); // WARN!!NO MAPPING NOW!
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
                // new_neighbors[newv].push_back(old2new[v]);
                new_neighbors[oldv].push_back(v); // WARN!!NO MAPPING NOW!
            }
        }
    }
    g.neighbors = new_neighbors;
    // g.vertex_num = n; // WARN!!NO MAPPING NOW!
    s = old2new[s];
    t = old2new[t];
}

bool st_biconnected_component(Graph &g, int &s, int &t)
{
    int n = g.vertexnum();
    std::vector<int> nr(n, 0);                  // dfs number
    std::vector<int> p(n, 0);                   // dfs parent
    std::vector<int> lowpt(n, 0);               // low point
    std::set<std::pair<int, int>> edge_visited; // mark edge visited
    std::vector<int> next_neighbor_idx(n, 0);   // mark the next neighbor idx to visit for each vertex
    int i = 1;                                  // current dfs number to assign
    int v = s;                                  // current vertex
    nr[s] = i;
    lowpt[s] = s;
    int w;

    std::vector<int> S = {s}; // stack of vertex
    bool contain_t_flag = 0;  // top componet at stack S contains t
    int s_degree = g.get_degree(s);
    while (p[v] || next_neighbor_idx[s] < s_degree)
    {
        // deep into until need to backtrack(i.e. when there is no unvisied edge for v)
        while (next_neighbor_idx[v] < g.get_degree(v))
        {
            w = g.get_neighbors(v)[next_neighbor_idx[v]];
            ++next_neighbor_idx[v];
            // vw is tree edge
            if (nr[w] == 0)
            {
                p[w] = v;
                nr[w] = ++i;
                lowpt[w] = i;
                // add to current component
                S.push_back(w);
                if (w == t)
                {
                    contain_t_flag = 1;
                }
                v = w;
            }
            // vw is back edge(fond)
            else
            {
                lowpt[v] = std::min(lowpt[v], nr[w]);
            }
#if DEBUG_LEVEL <= TRACE
            print_vectorln(S);
#endif
        }
        // v here can never be s: if then p[v]==0 && next_neighbor_idx[s] >= s_degree, which violate the condition of while(think of the case when entering the outer while v==s, v must advance and can not be s here)
        // backtrack
        if (p[v] != s)
        {
            // p[v] is not cut point
            if (lowpt[v] < nr[p[v]])
            {
                // update lowpt[pv] with lowpt[v]
                lowpt[p[v]] = std::min(lowpt[p[v]], lowpt[v]);
            }
            // p[v] is cut point
            else
            {
                // pop back top component(stack S down to v(v included), plus p[v])
                /*s and t may contain in separate block and the same time in the same block!
                (consider DFiso_eg 1 and 7, 7 in (0 2 4...)(not containing 1) as well as (1 7 8 10)(containing 1))
                thus, can not terminate early
                if (v != s && contain_t_flag) return 0;*/
#if DEBUG_LEVEL <= TRACE
                print_vectorln(S);
#endif
                while (S.back() != v)
                {
                    S.pop_back();
#if DEBUG_LEVEL <= TRACE
                    print_vectorln(S);
#endif
                }
                S.pop_back();
#if DEBUG_LEVEL <= TRACE
                print_vectorln(S);
#endif
                // contain_t_flag: top component containing t is poped, so clear flag
                // wrong: after poping top comp, the new top comp may also contain t too!(when t is a cut point between these 2 comp)
                // need to check too:
                // find t is in S or not, if not then clear flag
                if (contain_t_flag && std::find(S.begin(), S.end(), t) == S.end())
                {
                    contain_t_flag = 0;
                }
            }
        }
        // p[v]==s
        else
        {
#if DEBUG_LEVEL <= TRACE
            print_vectorln(S);
#endif
            // each component contains s
            // pop back top component(stack S down to v(v included), plus p[v])
            // found block(s), vertex in stack is V(block(s))
            if (contain_t_flag)
            {
                write_graph(g, s, t, S);
                return 1;
            }
            while (S.back() != v)
            {
                S.pop_back();
#if DEBUG_LEVEL <= TRACE
                print_vectorln(S);
#endif
            }
            S.pop_back();
#if DEBUG_LEVEL <= TRACE
            print_vectorln(S);
#endif
            if (contain_t_flag && std::find(S.begin(), S.end(), t) == S.end())
            {
                contain_t_flag = 0;
            }
        }
        v = p[v];
    }
    return 0;
}