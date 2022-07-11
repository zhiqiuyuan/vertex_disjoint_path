//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee,
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include "BoostTools.h"
/*注意，这里的实现是Graph的存储为：顶点从0开始连续id的*/
void to_boost_graph(graph_t &g, Graph &src)
{
    VID_TYPE n = src.vertexnum();
    for (VID_TYPE v = 0; v < n; ++v)
    {
        std::vector<VID_TYPE> nbs = src.get_neighbors(v);
        for (VID_TYPE nb : nbs)
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

bool st_biconnected_component(graph_t &g, VID_TYPE &s, VID_TYPE &t, std::vector<Edge> &before, std::vector<Edge> &after, bool print_comp_detail)
{
    boost::edge_component_t edge_component;
    boost::property_map<graph_t, boost::edge_component_t>::type component = get(edge_component, g);
    int num_comps = biconnected_components(g, component);
    if (print_comp_detail)
        print_with_colorln(BLUE, "boost num_comps:" + std::to_string(num_comps));
    boost::graph_traits<graph_t>::edge_iterator ei, ei_end;
    bool st_one_block = 0;
    // find which block s and t in
    std::set<VID_TYPE> snumv, tnumv;
    // component[*ei]: ei's comp number
    // source(*ei, g): size_t source vertex
    VID_TYPE tmps, tmpt;
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
    VID_TYPE snum = -1;
    for (VID_TYPE sn : snumv)
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
    VID_TYPE es, et;
    std::set<VID_TYPE> old; // vertices
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
    std::unordered_map<VID_TYPE, VID_TYPE> old2new;
    VID_TYPE i = 0;
    for (VID_TYPE oldv : old)
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

bool rec_st_biconnected_component(MemGraph &g, VID_TYPE &s, VID_TYPE &t, std::unordered_map<VID_TYPE, VID_TYPE> &new2old)
{
    VID_TYPE n = g.vertexnum();

    std::vector<VID_TYPE> num(n), par(n), low(n), art(n), stk;
    std::vector<VID_TYPE> comp;
    bool contain_s, contain_t;
    std::function<bool(VID_TYPE, VID_TYPE, VID_TYPE &)> dfs = [&](VID_TYPE u, VID_TYPE p, VID_TYPE &dfs_number)
    {
        par[u] = p;
        num[u] = low[u] = ++dfs_number;
        stk.push_back(u);

        for (VID_TYPE v : g.get_neighbors(u))
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
                            VID_TYPE tmpv = stk.back();
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
                            g.write_graph(comp);
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

    VID_TYPE dfs_number = 0;
    bool re = dfs(s, -1, dfs_number);
    print_with_colorln(BLUE, "vid\tlowpt:");
    for (VID_TYPE i = 0; i < n; ++i)
    {
        std::cout << i << "\t" << low[i] << std::endl;
    }
    print_with_colorln(BLUE, "vid\tdfsnumber:");
    for (VID_TYPE i = 0; i < n; ++i)
    {
        std::cout << i << "\t" << num[i] << std::endl;
    }
    print_with_colorln(BLUE, "vid\tpar:");
    for (VID_TYPE i = 0; i < n; ++i)
    {
        std::cout << i << "\t" << par[i] << std::endl;
    }
    return re;
}
