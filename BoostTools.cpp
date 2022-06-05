//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee,
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include "BoostTools.h"

void to_boost_graph(graph_t &g, Graph &src)
{
    int n = src.vertexnum();
    for (int v = 0; v < n; ++v)
    {
        std::vector<int> nbs = src.get_neighbors(v);
        for (int nb : nbs)
        {
            // for undirectedS, add(u,v) and add(v,u) would add 2 multiedge
            // nb<v then (nb,v) must already in
            if (nb >= v)
            {
                add_edge(v, nb, g);
            }
        }
    }
}

bool st_biconnected_component(graph_t &g, int &s, int &t, std::vector<Edge> &before, std::vector<Edge> &after, bool print_comp_detail)
{
    boost::edge_component_t edge_component;
    boost::property_map<graph_t, boost::edge_component_t>::type component = get(edge_component, g);
    std::size_t num_comps = biconnected_components(g, component);
    if (print_comp_detail)
        print_with_colorln(BLUE, "boost num_comps:" + std::to_string(num_comps));
    boost::graph_traits<graph_t>::edge_iterator ei, ei_end;
    bool st_one_block = 0;
    // find which block s and t in
    std::set<int> snumv, tnumv;
    // component[*ei]: ei's comp number
    // source(*ei, g): size_t source vertex
    int tmps, tmpt;
    std::vector<std::vector<Edge>> comps(num_comps);
    for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei)
    {
        tmps = source(*ei, g);
        tmpt = target(*ei, g);
        if (print_comp_detail)
            comps[component[*ei]].emplace_back(tmps, tmpt);
        if (tmps == s || tmpt == s)
        {
            snumv.insert(component[*ei]);
        }
        if (tmps == t || tmpt == t)
        {
            tnumv.insert(component[*ei]);
        }
    }
    if (print_comp_detail)
    {
        print_with_colorln(BLUE, "boost comps_detail:");
        for (int i = 0; i < num_comps; ++i)
        {
            std::cout << i << std::endl;
            for (auto e : comps[i])
            {
                std::cout << e.str() << std::endl;
            }
        }
        print_with_color(BLUE, "s in comps: ");
        print_setln(snumv);
        print_with_color(BLUE, "t in comps: ");
        print_setln(tnumv);
    }
    int snum = -1, tnum = -1;
    for (int sn : snumv)
    {
        if (tnumv.count(sn))
        {
            snum = sn;
            st_one_block = 1;
            break;
        }
    }
    //如果st不在同一个block中则不填写block(s)到before和after
    if (snum == -1)
    {
        return 0;
    }
    // sort edges
    before.clear();
    int es, et;
    std::set<int> old; // vertices
    for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei)
    {
        if (component[*ei] == snum)
        {
            es = source(*ei, g);
            et = target(*ei, g);
            old.insert(es);
            old.insert(et);
            if (es > et)
            {
                before.push_back({et, es});
            }
            else
            {
                before.push_back({es, et});
            }
        }
    }
    std::sort(before.begin(), before.end());
    auto it = std::unique(before.begin(), before.end());
    before.resize(it - before.begin());
    // mapping
    std::unordered_map<int, int> old2new;
    int i = 0;
    for (int oldv : old)
    {
        old2new[oldv] = i++;
    }
    s = old2new[s];
    t = old2new[t];
    after.clear();
    for (Edge e : before)
    {
        after.push_back({old2new[e.sv], old2new[e.tv]});
    }
    return st_one_block;
}

bool rec_st_biconnected_component(Graph &g, int &s, int &t, std::unordered_map<int, int> &new2old)
{
    int n = g.vertexnum();

    std::vector<int> num(n), par(n), low(n), art(n), stk;
    std::vector<int> comp;
    bool contain_s, contain_t;
    std::function<bool(int, int, int &)> dfs = [&](int u, int p, int &dfs_number)
    {
        par[u] = p;
        num[u] = low[u] = ++dfs_number;
        stk.push_back(u);

        for (int v : g.get_neighbors(u))
        {
            if (v != p)
            {
                if (!num[v])
                {
                    if (dfs(v, u, dfs_number))
                    {
                        return 1;
                    }
                    low[u] = std::min(low[u], low[v]);

                    if (low[v] >= num[u])
                    {
                        art[u] = (num[u] > 1 || num[v] > 2);
                        comp.clear();
                        comp.push_back(u);
                        contain_s = (u == s);
                        contain_t = (u == t);
                        while (comp.back() != v)
                        {
                            int tmpv = stk.back();
                            if (tmpv == s)
                            {
                                contain_s = 1;
                            }
                            if (tmpv == t)
                            {
                                contain_t = 1;
                            }
                            comp.push_back(tmpv);
                            stk.pop_back();
                        }
                        if (contain_s && contain_t)
                        {
                            write_graph(g, s, t, new2old, comp);
                            return 1;
                        }
                    }
                }
                else
                    low[u] = std::min(low[u], num[v]);
            }
        }
        return 0;
    };

    int dfs_number = 0;
    bool re = dfs(s, -1, dfs_number);
    print_with_colorln(BLUE, "vid\tlowpt:");
    for (int i = 0; i < n; ++i)
    {
        std::cout << i << "\t" << low[i] << std::endl;
    }
    print_with_colorln(BLUE, "vid\tdfsnumber:");
    for (int i = 0; i < n; ++i)
    {
        std::cout << i << "\t" << num[i] << std::endl;
    }
    print_with_colorln(BLUE, "vid\tpar:");
    for (int i = 0; i < n; ++i)
    {
        std::cout << i << "\t" << par[i] << std::endl;
    }
    return re;
}
