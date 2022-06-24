#include "TwoConnected.h"
#if TIME_KILL_ENABLE == 1
extern bool is_time_out;
#endif //#if TIME_KILL_ENABLE == 1
int solve_2VDPP(Graph &g, VID_TYPE s, VID_TYPE t)
{
    std::vector<VID_TYPE> path1, path2;
    std::back_insert_iterator<std::vector<VID_TYPE>> path1_back_it(path1), path2_back_it(path2);
    std::unordered_map<VID_TYPE, VID_TYPE> new2old;
#if DEBUG_LEVEL <= DEBUG
    print_with_colorln(DARK_YELLOW, "reduction: to b(s,t)");
#endif //#if DEBUG_LEVEL <= DEBUG
    int code = st_biconnected_component(g, s, t, new2old);
    if (code == 1)
    {
        std::vector<VID_TYPE> p1, p2;
        std::back_insert_iterator<std::vector<VID_TYPE>> p1_back_it(p1), p2_back_it(p2);
        code = solve_on_2connected(g, s, t, p1_back_it, p2_back_it);
        if (code == 1)
        {
            // translate path
            map_new2old(0, p1, path1_back_it, new2old);
            map_new2old(0, p2, path2_back_it, new2old);

            print_with_color(BLUE, "path1: ");
            print_vectorln(path1);
            print_with_color(BLUE, "path2: ");
            print_vectorln(path2);
            return code;
        }
    }
#if TIME_KILL_ENABLE == 1
    if (code == TIME_EXCEED_RESULT)
    {
        print_with_colorln(BLUE, "time limit exceed.");
        return code;
    }
#endif //#if TIME_KILL_ENABLE == 1
    print_with_colorln(BLUE, "no solution.");
    return code;
}

int solve_on_2connected(Graph &g, VID_TYPE s, VID_TYPE t, std::back_insert_iterator<std::vector<VID_TYPE>> path1_back_it, std::back_insert_iterator<std::vector<VID_TYPE>> path2_back_it)
{
    return remove_2vCut_containing_s(REMOVE_S, g, s, t, path1_back_it, path2_back_it);
}

// dfs number: from 1
// dfs parent and low point: identify by dfs number
int st_biconnected_component(Graph &g, VID_TYPE &s, VID_TYPE &t, std::unordered_map<VID_TYPE, VID_TYPE> &new2old)
{
    VID_TYPE n = g.vertexnum();
    std::vector<VID_TYPE> nr(n, 0);                // dfs number
    std::vector<VID_TYPE> p(n, 0);                 // dfs parent
    std::vector<VID_TYPE> lowpt(n, 0);             // low point
    std::vector<VID_TYPE> next_neighbor_idx(n, 0); // mark the next neighbor idx to visit for each vertex
    VID_TYPE i = 1;                                // current dfs number to assign
    VID_TYPE v = s;                                // current vertex
    nr[s] = i;
    lowpt[s] = i;
    VID_TYPE w;

    std::vector<VID_TYPE> S; // stack of vertex
    S.push_back(s);
    bool contain_t_flag = 0; // top componet at stack S contains t
    // while (p[v] || next_neighbor_idx[s] < s_degree)
    // while (p[v] || next_neighbor_idx[s] < s_degree || next_neighbor_idx[v] < g.get_degree(v))
    // while (next_neighbor_idx[v] < g.get_degree(v))
    while ((v == s && next_neighbor_idx[v] >= g.get_degree(v)) == 0)
    {
#if TIME_KILL_ENABLE == 1
        if (is_time_out)
            return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
       // deep into until need to backtrack(i.e. when there is no unvisied edge for v)
        while (next_neighbor_idx[v] < g.get_degree(v))
        {
            // std::cout << v << std::endl;
            w = g.get_neighbors(v)[next_neighbor_idx[v]];
            ++next_neighbor_idx[v];
            // vw is tree edge
            if (nr[w] == 0)
            {
                p[w] = v;
                ++i;
                nr[w] = i;
                lowpt[w] = i;
                // add to current component
                S.push_back(w);
                if (w == t)
                {
                    contain_t_flag = 1;
                }
                v = w;
            }
            // vw is back edge(fond) //note that w == p[v] also falls out of nr[w] == 0
            else if (w != p[v])
            {
                lowpt[v] = std::min(lowpt[v], nr[w]);
            }
#if DEBUG_LEVEL <= TRACE
            print_vectorln(S);
#endif
        }
        // v here can never be s: if then p[v]==0 && next_neighbor_idx[s] >= s_degree, which violate the condition of while(think of the case when entering the outer while v==s, v must advance and can not be s here)
        // backtrack
        // std::cout << "backtrack " << v << std::endl;
        if (v == s)
        {
            continue;
        }
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
                g.write_graph(s, t, new2old, S);
#if DEBUG_LEVEL <= TRACE
                print_with_colorln(BLUE, "vid\tlowpt:");
                for (VID_TYPE i = 0; i < n; ++i)
                {
                    std::cout << i << "\t" << lowpt[i] << std::endl;
                }
                print_with_colorln(BLUE, "vid\tdfsnumber:");
                for (VID_TYPE i = 0; i < n; ++i)
                {
                    std::cout << i << "\t" << nr[i] << std::endl;
                }
                print_with_colorln(BLUE, "vid\tpar:");
                for (VID_TYPE i = 0; i < n; ++i)
                {
                    std::cout << i << "\t" << p[i] << std::endl;
                }
#endif //#if DEBUG_LEVEL <= TRACE
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
        /*
        while (next_neighbor_idx[v] >= g.get_degree(v) && v != s)
        {
            lowpt[p[v]] = std::min(lowpt[p[v]], lowpt[v]);
            v = p[v];
        }
        */
    }
#if DEBUG_LEVEL <= TRACE
    print_with_colorln(BLUE, "vid\tlowpt:");
    for (VID_TYPE i = 0; i < n; ++i)
    {
        std::cout << i << "\t" << lowpt[i] << std::endl;
    }
    print_with_colorln(BLUE, "vid\tdfsnumber:");
    for (VID_TYPE i = 0; i < n; ++i)
    {
        std::cout << i << "\t" << nr[i] << std::endl;
    }
    print_with_colorln(BLUE, "vid\tpar:");
    for (VID_TYPE i = 0; i < n; ++i)
    {
        std::cout << i << "\t" << p[i] << std::endl;
    }
#endif //#if DEBUG_LEVEL <= TRACE
    return 0;
}
// remove_2vCut_containing_s 注意st交替调用怎么重组路径
int remove_2vCut_containing_s(Remove2VCutSel sel, Graph &g, VID_TYPE s, VID_TYPE t, std::back_insert_iterator<std::vector<VID_TYPE>> path1_back_it, std::back_insert_iterator<std::vector<VID_TYPE>> path2_back_it)
{
#if TIME_KILL_ENABLE == 1
    if (is_time_out)
        return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
    /*remove 2-vertex-cut containing s*/
    std::vector<VID_TYPE> s_neighbors = g.delete_vertex(s);
#if DEBUG_LEVEL <= TRACE
    g.print_graph();
    std::cout << std::endl;
#endif                                                          //#if DEBUG_LEVEL <= TRACE
    std::vector<bctreeNode> bctree;                             // idx is comp number too, parent pointer representation
    int t_comp;                                                 // if t is not cutpoint: which comp is block(t); if is cutpoint: we set -1
    std::vector<std::set<VID_TYPE>> comps_V;                    // comp number->comps vertex set
    std::vector<std::vector<int>> s_adj_comp;                   // idx correspond to s_neighbors: which comp(s) is s_neighbors[i] in(note that if s_neighbors[i] is cutpoint, more than 1 located comp; else 1 located comp)
    std::unordered_map<int, std::vector<VID_TYPE>> comp2s_nbrs; // if one comp contains one s_neighbors, record it
    bool t_is_cut_point;
    int code = build_bctree(g, t, s_neighbors, bctree, t_comp, comps_V, s_adj_comp);
#if TIME_KILL_ENABLE == 1
    if (is_time_out)
        return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
    t_is_cut_point = code;
#if DEBUG_LEVEL <= TRACE
    print_with_colorln(BLUE, "t_is_cut_point:" + std::to_string(t_is_cut_point));
    print_bctree_vector(bctree);
    print_comps_V(comps_V);
    print_s_adj_comp(s_adj_comp, s_neighbors);
    print_with_colorln(BLUE, "t_comp:" + std::to_string(t_comp));
    std::cout << std::endl;
#endif //#if DEBUG_LEVEL <= TRACE
    if (comps_V.size() > 1)
    {
        // s_adj_comp->comp2s_nbrs
        for (size_t i = 0; i < s_adj_comp.size(); ++i)
        {
            for (int comp : s_adj_comp[i])
                comp2s_nbrs[comp].push_back(s_neighbors[i]);
        }
        /*below: "s" "t" means params*/
        if (t_is_cut_point) //此分支测试近似无误 path1: 0 2 9 path2: 0 5 9
        {
            int compu, compv;
            VID_TYPE u, v;
            no_common_ancestor(compu, compv, u, v, s_adj_comp, bctree, t_comp);
            u = s_neighbors[u];
            v = s_neighbors[v];
            std::vector<VID_TYPE> u2t, v2t;
            std::back_insert_iterator<std::vector<VID_TYPE>> u2t_back_it(u2t), v2t_back_it(v2t);
            // "t"(i.e. root of bctree) is cutpoint,thus "t" is the destination of the last path segment
            get_parent_path(compu, u, t, u2t_back_it, g, bctree, comps_V);
            get_parent_path(compv, v, t, v2t_back_it, g, bctree, comps_V);
#if DEBUG_LEVEL <= TRACE
            print_vectorln(u2t);
            print_vectorln(v2t);
#endif //#if DEBUG_LEVEL <= TRACE

            //"s"->u-->"t"
            //"s"->v-->"t"
            if (sel == REMOVE_S)
            {
                path1_back_it = s; // Using the assignment operator on the back_insert_iterator (both while being dereferenced or not), causes the container to expand by one element, which is initialized to the value assigned.
                std::copy(u2t.begin(), u2t.end(), path1_back_it);
                path2_back_it = s;
                std::copy(v2t.begin(), v2t.end(), path2_back_it);
            }
            //"t"-->u->"s"
            //"t"-->v->"s"
            else
            {
                // NOTE: std::copy(.cbegin(), .cend()) takes the same effect as copy(.begin(), .end())!
                std::reverse_copy(u2t.begin(), u2t.end(), path1_back_it);
                path1_back_it = s;
                std::reverse_copy(v2t.begin(), v2t.end(), path2_back_it);
                path2_back_it = s;
            }
            return 1;
        }
        // "t" is not cut point
        else
        {
            if (comp2s_nbrs.count(t_comp))
            {
                std::vector<VID_TYPE> link_ends = comp2s_nbrs[t_comp];
                if (link_ends.size() >= 2) // u v
                {
                    // new g: b("t"), edge("s",u), edge("s",v)
                    std::unordered_map<VID_TYPE, VID_TYPE> new2old;
                    g.recover_vertex(s, {link_ends[0], link_ends[1]});
                    comps_V[t_comp].insert(s);
                    // int old_s = s;

#if DEBUG_LEVEL <= DEBUG
                    print_with_colorln(DARK_YELLOW, "reduction: in remove_2vCut_containing_" + std::string((sel == REMOVE_S ? "s" : "t")));
#endif //#if DEBUG_LEVEL <= DEBUG
                    g.write_graph(s, t, new2old, comps_V[t_comp]);
                    // comps_V[t_comp].erase(old_s);

                    std::vector<VID_TYPE> p1, p2;
                    std::back_insert_iterator<std::vector<VID_TYPE>> p1_back_it(p1), p2_back_it(p2);
                    int code;
                    if (sel == REMOVE_S)
                        code = remove_2vCut_containing_s(REMOVE_T, g, t, s, p1_back_it, p2_back_it);
                    else
                        code = remove_2vCut_containing_s(REMOVE_S, g, t, s, p1_back_it, p2_back_it);
#if TIME_KILL_ENABLE == 1
                    if (is_time_out)
                        return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
                    if (code)
                    {
#if DEBUG_LEVEL <= TRACE
                        // sel == REMOVE_S: new"s"->new"t"
                        // sel == REMOVE_T: new"t"->new"s"
                        print_vectorln(p1);
                        print_vectorln(p2);
#endif //#if DEBUG_LEVEL <= TRACE

                        // translate path
                        map_new2old(0, p1, path1_back_it, new2old);
                        map_new2old(0, p2, path2_back_it, new2old);
                    }
                    return code;
                }
                else // u
                {
                    // new g: b("t"), edge("s",u), edge("s",v)
                    // in which v is: start from any "s"_adj_comp(except b("t")) start_comp (parent path), the cut point adjancent to "t"
                    VID_TYPE u = link_ends[0];
                    int start_comp;
                    // must exist: "s" has only 1 link to b("t"), given original G 2connected, there must be another link from "s" to other comp
                    for (auto p : comp2s_nbrs)
                    {
                        if (p.first != t_comp)
                        {
                            start_comp = p.first;
                            break;
                        }
                    }
                    // "s"->k-(parent path)->v-(t_comp)->"t"
                    VID_TYPE k = comp2s_nbrs[start_comp][0];
                    std::vector<VID_TYPE> k2v, k2t_cutpoint;
                    std::back_insert_iterator<std::vector<VID_TYPE>> k2v_back_it(k2v), k2t_cutpoint_back_it(k2t_cutpoint);
                    get_cut_point_path(start_comp, k, t, bctree, k2t_cutpoint_back_it);
                    VID_TYPE v = k2t_cutpoint[k2t_cutpoint.size() - 2]; // last is t, the second last is v (2nd last must exist, otherwise this s_nbr is located in b(t), which contradict with only one link from s to b(t))
                    get_parent_path(start_comp, k, v, k2v_back_it, g, bctree, comps_V);
#if DEBUG_LEVEL <= TRACE
                    print_vectorln(k2v);
#endif //#if DEBUG_LEVEL <= TRACE

                    // new g: b("t"), edge("s",u), edge("s",v)
                    std::unordered_map<VID_TYPE, VID_TYPE> new2old;
                    g.recover_vertex(s, {u, v});
                    comps_V[t_comp].insert(s);
                    // VID_TYPE old_s = s;

#if DEBUG_LEVEL <= DEBUG
                    print_with_colorln(DARK_YELLOW, "reduction: in remove_2vCut_containing_" + std::string((sel == REMOVE_S ? "s" : "t")));
#endif //#if DEBUG_LEVEL <= DEBUG
                    g.write_graph(s, t, new2old, comps_V[t_comp]);
                    // comps_V[t_comp].erase(old_s);
                    std::vector<VID_TYPE> p1, p2;
                    std::back_insert_iterator<std::vector<VID_TYPE>> p1_back_it(p1), p2_back_it(p2);
                    int code;
                    if (sel == REMOVE_S)
                        code = remove_2vCut_containing_s(REMOVE_T, g, t, s, p1_back_it, p2_back_it);
                    else
                        code = remove_2vCut_containing_s(REMOVE_S, g, t, s, p1_back_it, p2_back_it);

#if TIME_KILL_ENABLE == 1
                    if (is_time_out)
                        return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
                    if (code)
                    {
#if DEBUG_LEVEL <= TRACE
                        print_vectorln(k2v);
                        print_vectorln(p1);
                        print_vectorln(p2);
#endif //#if DEBUG_LEVEL <= TRACE

                        // assemble and translate path
                        // k2v: "s"->k-(parent path)->v-(t_comp)->"t"

                        // sel == REMOVE_S: p1p2: new"s"->new"t"
                        // path1: "s"->k-->v-->"t" (p1/2 is "s"->v-->"t") "s"+k2v+p1/2[2:]
                        // path2: "s"->u-->"t" (p2/1 is "s"->u-->"t") p2/1
                        if (sel == REMOVE_S)
                        {
                            path1_back_it = new2old[s];
                            std::copy(k2v.begin(), k2v.end(), path1_back_it);
                            if (new2old[p1[1]] == v)
                            {
                                map_new2old(2, p1, path1_back_it, new2old);
                                map_new2old(0, p2, path2_back_it, new2old);
                            }
                            else
                            {
                                map_new2old(2, p2, path1_back_it, new2old);
                                map_new2old(0, p1, path2_back_it, new2old);
                            }
                        }
                        // sel == REMOVE_T: p1p2: new"t"->new"s"
                        // path1: "t"-->v-->k->"s" (p1/2 is "t"-->v->"s") p1/2[:-2]+k2v.rev+"s"
                        // path2: "t"-->u->"s" (p2/1 is "t"-->u->"s") p2/1
                        else
                        {
                            if (new2old[p1[1]] == v)
                            {
                                p1.pop_back(); // pop "s"
                                p1.pop_back(); // pop v
                                map_new2old(0, p1, path1_back_it, new2old);
                                map_new2old(0, p2, path2_back_it, new2old);
                            }
                            else
                            {
                                p2.pop_back(); // pop "s"
                                p2.pop_back(); // pop v
                                map_new2old(0, p2, path1_back_it, new2old);
                                map_new2old(0, p1, path2_back_it, new2old);
                            }
                            std::reverse_copy(k2v.begin(), k2v.end(), path1_back_it);
                            path1_back_it = new2old[s];
                        }
                    }
                    return code;
                }
            }
            // "s" no link to b("t")
            else
            {
                // new g: b("t"), edge("s",a), edge("s",b)
                int start_comp1, start_comp2;
                VID_TYPE k1, k2;
                no_common_ancestor(start_comp1, start_comp2, k1, k2, s_adj_comp, bctree, t_comp);
                k1 = s_neighbors[k1];
                k2 = s_neighbors[k2];
                std::vector<VID_TYPE> k12a, k22b, k12t_cutpoint, k22t_cutpoint;
                std::back_insert_iterator<std::vector<VID_TYPE>> k12a_back_it(k12a), k22b_back_it(k22b), k12t_cutpoint_back_it(k12t_cutpoint), k22t_cutpoint_back_it(k22t_cutpoint);
                get_cut_point_path(start_comp1, k1, t, bctree, k12t_cutpoint_back_it);
                VID_TYPE a = k12t_cutpoint[k12t_cutpoint.size() - 2]; // last is t, the second last is a
                get_parent_path(start_comp1, k1, a, k12a_back_it, g, bctree, comps_V);
                get_cut_point_path(start_comp2, k2, t, bctree, k22t_cutpoint_back_it);
                VID_TYPE b = k22t_cutpoint[k22t_cutpoint.size() - 2]; // last is t, the second last is b
                get_parent_path(start_comp2, k2, b, k22b_back_it, g, bctree, comps_V);

#if DEBUG_LEVEL <= TRACE
                print_vectorln(k12a);
                print_vectorln(k22b);
#endif //#if DEBUG_LEVEL <= TRACE

                // new g: b("t"), edge("s",a), edge("s",b)
                std::unordered_map<VID_TYPE, VID_TYPE> new2old;
                g.recover_vertex(s, {a, b});
                comps_V[t_comp].insert(s);
                // VID_TYPE old_s = s;

#if DEBUG_LEVEL <= DEBUG
                print_with_colorln(DARK_YELLOW, "reduction: in remove_2vCut_containing_" + std::string((sel == REMOVE_S ? "s" : "t")));
#endif //#if DEBUG_LEVEL <= DEBUG
                g.write_graph(s, t, new2old, comps_V[t_comp]);
                // comps_V[t_comp].erase(old_s);
                std::vector<VID_TYPE> p1, p2;
                std::back_insert_iterator<std::vector<VID_TYPE>> p1_back_it(p1), p2_back_it(p2);
                int code;
                if (sel == REMOVE_S)
                    code = remove_2vCut_containing_s(REMOVE_T, g, t, s, p1_back_it, p2_back_it);
                else
                    code = remove_2vCut_containing_s(REMOVE_S, g, t, s, p1_back_it, p2_back_it);

#if TIME_KILL_ENABLE == 1
                if (is_time_out)
                    return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
                if (code)
                {
#if DEBUG_LEVEL <= TRACE
                    print_vectorln(k12a);
                    print_vectorln(k22b);
                    print_vectorln(p1);
                    print_vectorln(p2);
#endif //#if DEBUG_LEVEL <= TRACE

                    // assemble and translate path
                    // k12a:"s"->k1-(parent path)->a-(t_comp)->"t"
                    // k22b:"s"->k2-(parent path)->b-(t_comp)->"t"

                    // sel == REMOVE_S: p1p2: new"s"->new"t"
                    // path1: "s"->k1-->a-->"t" (p1/2 is "s"->a-->"t") "s"+k12a+p1/2[2:]
                    // path2: "s"->k2-->b-->"t" (p2/1 is "s"->b-->"t") "s"+k22b+p2/1[2:]
                    if (sel == REMOVE_S)
                    {
                        path1_back_it = new2old[s];
                        std::copy(k12a.begin(), k12a.end(), path1_back_it);
                        path2_back_it = new2old[s];
                        std::copy(k22b.begin(), k22b.end(), path2_back_it);
                        if (new2old[p1[1]] == a)
                        {
                            map_new2old(2, p1, path1_back_it, new2old);
                            map_new2old(2, p2, path2_back_it, new2old);
                        }
                        else
                        {
                            map_new2old(2, p2, path1_back_it, new2old);
                            map_new2old(2, p1, path2_back_it, new2old);
                        }
                    }
                    // sel == REMOVE_T: p1p2: new"t"->new"s"
                    // path1: "t"-->a-->k1->"s" (p1/2 is "t"-->a->"s") p1/2[:-2]+k12a.rev+"s"
                    // path2: "t"-->b-->k2->"s" (p2/1 is "t"-->b->"s") p2/1[:-2]+k22b.rev+"s"
                    else
                    {
                        p1.pop_back(); // pop "s"
                        p1.pop_back(); // pop a/b
                        p2.pop_back(); // pop "s"
                        p2.pop_back(); // pop b/a
                        if (new2old[p1[1]] == a)
                        {
                            map_new2old(0, p1, path1_back_it, new2old);
                            map_new2old(0, p2, path2_back_it, new2old);
                        }
                        else
                        {
                            map_new2old(0, p2, path1_back_it, new2old);
                            map_new2old(0, p1, path2_back_it, new2old);
                        }
                        std::reverse_copy(k12a.begin(), k12a.end(), path1_back_it);
                        path1_back_it = new2old[s];
                        std::reverse_copy(k22b.begin(), k22b.end(), path2_back_it);
                        path2_back_it = new2old[s];
                    }
                }
                return code;
            }
        }
    }
    // comps_V.size() <= 1: no 2-vertex-cut containing s, recover s neighbors
    else
    {
        g.recover_vertex(s, s_neighbors);
#if DEBUG_LEVEL <= TRACE
        g.print_graph();
        std::cout << std::endl;
#endif
        /*remove 2-vertex-cut containing t*/
        if (sel == REMOVE_S)
        {
            /*初始调用是调用"删含s的"，则sel == REMOVE_S时 此次删除之后图不变仍然需要继续对含t的进行删除*/
            return remove_2vCut_containing_s(REMOVE_T, g, t, s, path1_back_it, path2_back_it);
        }
        /*进入下一阶段（G中此时的2vertexcut都不含s和t）*/
        else
        {
            /*sel==REMOVE_T则必然有前一次调用且是sel==REMOVE_S的调用
            调用此次时（进入调用之前）图不含含s的，此次调用进行删含t的则删完之后图不含含t的，
            而删完之后图不变则仍然不含含s的，则reduce结束
            */
#if DEBUG_LEVEL <= DEBUG
            print_with_colorln(DARK_YELLOW, "remove all 2vertexcut containing s or t done. goto remove_2vCut");
#endif //#if DEBUG_LEVEL <= DEBUG
            return remove_2vCut(g, t, s, path1_back_it, path2_back_it);
        }
    }
}

#if DEBUG_LEVEL <= TRACE
void print_cutpoint2comp(std::unordered_map<VID_TYPE, std::vector<int>> cutpoint2comp)
{
    print_with_colorln(BLUE, "cutpoint:\thanging comp");
    for (auto p : cutpoint2comp)
    {
        std::cout << p.first << ":\t";
        for (int comp : p.second)
        {
            std::cout << comp << " ";
        }
        std::cout << std::endl;
    }
}
#endif // DEBUG_LEVEL<=TRACE
// insert v into comp k
// update comps_V, s_adj_comp, bctree
inline void insert_v_into_comp(std::vector<std::set<VID_TYPE>> &comps_V, std::vector<std::vector<int>> &s_adj_comp, std::vector<bctreeNode> &bctree, int k, VID_TYPE v, std::unordered_map<VID_TYPE, int> &snbrs2idx, std::unordered_map<VID_TYPE, std::vector<int>> cutpoint2comp)
{
    comps_V[k].insert(v);
    // if v is snbr
    // record v's located comp(i.e. comp k) to snbr
    if (snbrs2idx.count(v))
    {
        s_adj_comp[snbrs2idx[v]].push_back(k);
    }
#if DEBUG_LEVEL <= TRACE
    print_cutpoint2comp(cutpoint2comp);
    std::cout << std::endl;
#endif
    // pick comp children for comp k(comps that hanging at all vertex in comp k except the "parent cut point")
    if (cutpoint2comp.count(v))
    {
        // comp->v-->t
        for (int comp : cutpoint2comp[v])
        {
            bctree[comp].parent = k;
        }
    }
}
// insert "parent cut point" v into comp k
//  update comps_V, s_adj_comp
inline void insert_parentcutv_into_comp(std::vector<std::set<VID_TYPE>> &comps_V, std::vector<std::vector<int>> &s_adj_comp, int k, VID_TYPE v, std::unordered_map<VID_TYPE, int> &snbrs2idx)
{
    comps_V[k].insert(v);
    // if v is snbr
    // record v's located comp(i.e. comp k) to snbr
    if (snbrs2idx.count(v))
    {
        s_adj_comp[snbrs2idx[v]].push_back(k);
    }
}

int build_bctree(Graph &g, VID_TYPE t, const std::vector<VID_TYPE> s_neighbors, std::vector<bctreeNode> &bctree, int &t_comp, std::vector<std::set<VID_TYPE>> &comps_V, std::vector<std::vector<int>> &s_adj_comp)
{
    std::unordered_map<VID_TYPE, int> snbrs2idx; // to idx in s_neighbors
    for (size_t i = 0; i < s_neighbors.size(); ++i)
    {
        snbrs2idx[s_neighbors[i]] = i;
    }
    s_adj_comp.resize(s_neighbors.size());
    for (auto &vec : s_adj_comp)
    {
        vec.clear();
    }
    bool t_is_cut_point = 0;

    VID_TYPE n = g.vertexnum();
    std::vector<VID_TYPE> nr(n, 0);                // dfs number
    std::vector<VID_TYPE> p(n, 0);                 // dfs parent
    std::vector<VID_TYPE> lowpt(n, 0);             // low point
    std::vector<VID_TYPE> next_neighbor_idx(n, 0); // mark the next neighbor idx to visit for each vertex
    VID_TYPE i = 1;                                // current dfs number to assign
    int k = 0;                                     // current comp number
    VID_TYPE v = t;                                // current vertex
    nr[v] = i;
    lowpt[v] = i;
    VID_TYPE w;
    std::unordered_map<VID_TYPE, std::vector<int>> cutpoint2comp; // bctreeNode(.second[i]).cut_point is .first
    // all comps that hanging at each cutpoint. used for determine parent comp for bctree
    std::set<int> t_comps;

    std::vector<VID_TYPE> S; // stack of vertex
    S.push_back(v);
    // while (p[v] || next_neighbor_idx[t] < t_degree)
    // while (next_neighbor_idx[v] < g.get_degree(v))
    while ((v == t && next_neighbor_idx[v] >= g.get_degree(v)) == 0)
    {
#if TIME_KILL_ENABLE == 1
        if (is_time_out)
            return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
       // deep into until need to backtrack(i.e. when there is no unvisied edge for v)
        while (next_neighbor_idx[v] < g.get_degree(v))
        {
            w = g.get_neighbors(v)[next_neighbor_idx[v]];
            ++next_neighbor_idx[v];
            // vw is tree edge
            if (nr[w] == 0)
            {
                p[w] = v;
                ++i;
                nr[w] = i;
                lowpt[w] = i;
                // add to current component
                S.push_back(w);
                v = w;
            }
            // vw is back edge(fond)
            else if (w != p[v])
            {
                lowpt[v] = std::min(lowpt[v], nr[w]);
            }
#if DEBUG_LEVEL <= TRACE
            print_vectorln(S);
#endif
        }
        // v here can never be s: if then p[v]==0 && next_neighbor_idx[s] >= s_degree, which violate the condition of while(think of the case when entering the outer while v==s, v must advance and can not be s here)
        // backtrack
        if (v == t)
        {
            continue;
        }
        if (p[v] != t)
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
#if DEBUG_LEVEL <= TRACE
                print_vectorln(S);
#endif
                comps_V.resize(k + 1);
                while (S.back() != v)
                {
                    VID_TYPE tmpv = S.back();
                    insert_v_into_comp(comps_V, s_adj_comp, bctree, k, tmpv, snbrs2idx, cutpoint2comp);
                    S.pop_back();
#if DEBUG_LEVEL <= TRACE
                    print_vectorln(S);
#endif
                }
                insert_v_into_comp(comps_V, s_adj_comp, bctree, k, v, snbrs2idx, cutpoint2comp);
                S.pop_back();
                insert_parentcutv_into_comp(comps_V, s_adj_comp, k, p[v], snbrs2idx);
#if DEBUG_LEVEL <= TRACE
                print_comps_V(comps_V);
                std::cout << std::endl;
#endif
                bctree.emplace_back(p[v]); // comp k's cut point is p[v]
                cutpoint2comp[p[v]].push_back(k);
                ++k;
#if DEBUG_LEVEL <= TRACE
                print_vectorln(S);
#endif
            }
        }
        // p[v]==t
        else
        {
#if DEBUG_LEVEL <= TRACE
            print_vectorln(S);
#endif
            // each component contains t
            // pop back top component(stack S down to v(v included), plus p[v])

            // when t is cut point: think of case: all vertices has been visited, but there is still edge for t to visit, then the travel will go through those edge and backtrack instantly,
            // when backtrack from those neighbors, control will reach here, there is definitely no more comps, but if we don't set the condition here, we would go for create another comp!
            if (std::find(S.begin(), S.end(), v) != S.end())
            // if (next_neighbor_idx[t] < g.get_degree(t))
            {
                t_comps.insert(k);
                comps_V.resize(k + 1);
                while (S.back() != v)
                {
                    VID_TYPE tmpv = S.back();
                    insert_v_into_comp(comps_V, s_adj_comp, bctree, k, tmpv, snbrs2idx, cutpoint2comp);
                    S.pop_back();
#if DEBUG_LEVEL <= TRACE
                    print_vectorln(S);
#endif
                }
                insert_v_into_comp(comps_V, s_adj_comp, bctree, k, v, snbrs2idx, cutpoint2comp);
                S.pop_back();
                insert_parentcutv_into_comp(comps_V, s_adj_comp, k, t, snbrs2idx);
#if DEBUG_LEVEL <= TRACE
                print_comps_V(comps_V);
                std::cout << std::endl;
#endif
                bctree.emplace_back(p[v]); // comp k's cut point is p[v]
                cutpoint2comp[p[v]].push_back(k);
                ++k;
#if DEBUG_LEVEL <= TRACE
                print_vectorln(S);
#endif
            }
        }
        v = p[v];
        /*
        while (next_neighbor_idx[v] >= g.get_degree(v) && v != t)
        {
            lowpt[p[v]] = std::min(lowpt[p[v]], lowpt[v]);
            v = p[v];
        }
        */
    }
    t_is_cut_point = (t_comps.size() > 1);
    if (t_is_cut_point)
    {
        t_comp = -1;
    }
    else
    {
        t_comp = k - 1; // the last comp is block(t)
    }
    comps_V.resize(k);
#if DEBUG_LEVEL <= TRACE
    print_with_colorln(BLUE, "vid\tlowpt:");
    for (VID_TYPE i = 0; i < n; ++i)
    {
        std::cout << i << "\t" << lowpt[i] << std::endl;
    }
    print_with_colorln(BLUE, "vid\tdfsnumber:");
    for (VID_TYPE i = 0; i < n; ++i)
    {
        std::cout << i << "\t" << nr[i] << std::endl;
    }
    print_with_colorln(BLUE, "vid\tpar:");
    for (VID_TYPE i = 0; i < n; ++i)
    {
        std::cout << i << "\t" << p[i] << std::endl;
    }
#endif //#if DEBUG_LEVEL <= TRACE
    return t_is_cut_point;
}
#if DEBUG_LEVEL <= DEBUG
void print_bctree_vector(const std::vector<bctreeNode> &bctree)
{
    print_with_colorln(BLUE, "bctree");
    std::cout << "comp\tparent\tcutpoint\n";
    for (size_t i = 0; i < bctree.size(); ++i)
    {
        std::cout << i << "\t" << bctree[i].parent << "\t" << bctree[i].cut_point << std::endl;
    }
}
void print_comps_V(const std::vector<std::set<VID_TYPE>> &comps_V)
{
    print_with_colorln(BLUE, "comps_V");
    for (size_t i = 0; i < comps_V.size(); ++i)
    {
        std::cout << "comp " << i << ":" << std::endl;
        for (VID_TYPE v : comps_V[i])
        {
            std::cout << v << " ";
        }
        std::cout << std::endl;
    }
}
void print_s_adj_comp(const std::vector<std::vector<int>> &s_adj_comp, const std::vector<VID_TYPE> &s_neighbors)
{
    print_with_colorln(BLUE, "s_adj_comp");
    std::cout << "snbr:\tadj_comps" << std::endl;
    for (size_t i = 0; i < s_adj_comp.size(); ++i)
    {
        std::cout << s_neighbors[i] << ":\t";
        for (int comp : s_adj_comp[i])
        {
            std::cout << comp << " ";
        }
        std::cout << std::endl;
    }
}
#endif //#if DEBUG_LEVEL <= DEBUG

int get_root_comp(int comp, const std::vector<bctreeNode> &bctree, int t_comp)
{
    assert(comp >= 0 && comp < (int)bctree.size());
    int c = comp;
    while (bctree[c].parent != t_comp && bctree[c].parent != -1)
    // t_comp might not be -1. in this case, when bctree[c].parent == -1, we should stop(otherwise next while loop will do bctree[-1])
    {
        c = bctree[c].parent;
    }
    return c;
}
void no_common_ancestor(int &compu, int &compv, VID_TYPE &uidx, VID_TYPE &vidx, const std::vector<std::vector<int>> &s_adj_comp, const std::vector<bctreeNode> &bctree, int t_comp)
{
    std::unordered_map<int, std::vector<int>> rootcomp2comp, rootcomp2snbridx;
    int s_nbrs_num = s_adj_comp.size();
    for (int i = 0; i < s_nbrs_num; ++i)
    {
        if (s_adj_comp[i].size() == 1)
        {
            int comp = s_adj_comp[i][0];
            int root = get_root_comp(comp, bctree, t_comp);
            // there exist root encounted before that differ from this root
            if (rootcomp2comp.size() - rootcomp2comp.count(root) >= 1)
            {
                compv = comp;
                vidx = i;
                for (auto p : rootcomp2comp)
                {
                    if (p.first != root)
                    {
                        compu = p.second[0];
                        uidx = rootcomp2snbridx[p.first][0];
                        break;
                    }
                }
                return;
            }
            rootcomp2comp[root].push_back(comp);
            rootcomp2snbridx[root].push_back(i);
        }
        else if (s_adj_comp[i].size() > 1)
        {
            std::vector<int> roots, comps;
            int root;
            for (int comp : s_adj_comp[i])
            {
                root = get_root_comp(comp, bctree, t_comp);
                // there exist root encounted before that differ from this root
                // comp cannot be of the same s_nbr, because we examine just by one pass
                if (rootcomp2comp.size() - rootcomp2comp.count(root) >= 1)
                {
                    compv = comp;
                    vidx = i;
                    for (auto p : rootcomp2comp)
                    {
                        if (p.first != root)
                        {
                            compu = p.second[0];
                            uidx = rootcomp2snbridx[p.first][0];
                            break;
                        }
                    }
                    return;
                }
                roots.push_back(root);
                comps.push_back(comp);
            }
            // add-to-map are packed together for one snbr:to avoid return 2 comps satisfiying "no common ancestor" with the same corresponding snbr
            for (size_t j = 0; j < roots.size(); ++j)
            {
                rootcomp2comp[roots[j]].push_back(comps[j]);
                rootcomp2snbridx[roots[j]].push_back(i);
            }
        }
    }
    // impossible to reach here: because
    // impossible: all s_nbrs has one root(except b(t) if t is not cutpoint) in bctree
    //(if true, the original graph turns disconnected after removing one vertex(t or the cut point connecting b(t) and "this root"), which contradicts biconnectivity)
    compu = -1;
    compv = -1;
    uidx = -1;
    uidx = -1;
    Assert(1, "all s_nbrs's located comp has the same root(except b(t) if t is not cutpoint) in bctree,\n which is impossible for a biconnected graph after removing s.\n");
}

// only visit vertices in vset in g
// s!=t: add [s,t) to path_back_it
// s==t:do nothing
bool bibfs_get_path(VID_TYPE sv, VID_TYPE tv, std::back_insert_iterator<std::vector<VID_TYPE>> path_back_it, const std::set<VID_TYPE> &vset, Graph &g)
{
    if (sv == tv)
    {
        return 1;
    }
    std::deque<VID_TYPE> path;
    std::set<VID_TYPE> s_visited; // mark as visited; mark when pushing into queue
    s_visited.insert(sv);
    std::set<VID_TYPE> t_visited;
    t_visited.insert(tv);
    std::queue<VID_TYPE> sq, tq;
    sq.push(sv);
    tq.push(tv);
    VID_TYPE curr;
    VID_TYPE joint = -1;
    // s->joint: only one prec
    // joint->t: only one succ
    std::map<VID_TYPE, VID_TYPE> prec, succ;
    while (sq.empty() == 0 && tq.empty() == 0 && joint == -1)
    {
#if DEBUG_LEVEL <= TRACE
        std::cout << "sq:";
        print_queueln(sq);
        std::cout << "tq:";
        print_queueln(tq);
#endif //#if DEBUG_LEVEL <= TRACE
       // s->
        if (sq.size() < tq.size())
        {
            curr = sq.front();
            sq.pop();
            std::vector<VID_TYPE> neighbors = g.get_neighbors(curr);
#if DEBUG_LEVEL <= TRACE
            std::cout << curr << " neighbors:";
            print_vectorln(neighbors);
#endif //#if DEBUG_LEVEL <= TRACE
            for (VID_TYPE n : neighbors)
            {
                if (vset.count(n))
                {
                    if (t_visited.count(n))
                    {
                        prec[n] = curr;
                        joint = n;
                        break;
                    }
                    if (s_visited.count(n) == 0)
                    {
                        sq.push(n);
                        prec[n] = curr;
                        s_visited.insert(n);
                    }
                }
            }
        }
        // t<-
        else
        {
            curr = tq.front();
            tq.pop();
            std::vector<VID_TYPE> neighbors = g.get_neighbors(curr);
#if DEBUG_LEVEL <= TRACE
            std::cout << curr << " neighbors:";
            print_vectorln(neighbors);
#endif //#if DEBUG_LEVEL <= TRACE
            for (VID_TYPE n : neighbors)
            {
                if (vset.count(n))
                {
                    if (s_visited.count(n))
                    {
                        succ[n] = curr;
                        joint = n;
                        break;
                    }
                    if (t_visited.count(n) == 0)
                    {
                        tq.push(n);
                        succ[n] = curr;
                        t_visited.insert(n);
                    }
                }
            }
        }
    }
    // trace path
    if (joint != -1)
    {
        curr = joint;
        // tv is not included in path
        if (curr != tv)
        {
            path.push_front(curr);
        }
        // s->joint: only one prec
        // need testing prec.count(curr), for maybe joint has no prec(i.e. sv)
        while (prec.count(curr) && prec[curr] != sv)
        {
            curr = prec[curr];
            path.push_front(curr);
        }
        if (prec.count(curr))
        {
            path.push_front(sv); // add sv
        }
        // joint->t: only one succ
        curr = joint;
        // need testing succ.count(curr), for maybe joint has no succ(i.e. tv)
        while (succ.count(curr) && succ[curr] != tv)
        {
            curr = succ[curr];
            path.push_back(curr);
        }
        std::copy(path.begin(), path.end(), path_back_it);
#if DEBUG_LEVEL <= TRACE
        print_dequeln(path);
#endif //#if DEBUG_LEVEL <= TRACE
        return 1;
    }
    return 0;
}

void get_parent_path(int start_comp, VID_TYPE start_v, VID_TYPE end_cut_point, std::back_insert_iterator<std::vector<VID_TYPE>> path_back_it, Graph &g, const std::vector<bctreeNode> &bctree, const std::vector<std::set<VID_TYPE>> &comps_V)
{
    // each segment: if s!=t: add[s,t); s==t:add nothing
    VID_TYPE s = start_v, t = bctree[start_comp].cut_point;
    // VisitLimitedVset instance(g);
    while (t != end_cut_point && start_comp >= 0)
    {
        if (bibfs_get_path(s, t, path_back_it, comps_V[start_comp], g) == 0)
        {
            Assert(1, std::to_string(s) + "->" + std::to_string(t) + " can not find path in biconnected comp!\n");
        }
        s = t;
        start_comp = bctree[start_comp].parent;
        if (start_comp < 0)
        {
            break;
        }
        t = bctree[start_comp].cut_point;
    }
    // last segment
    // t==end_cut_point
    if (start_comp >= 0)
    {
        if (bibfs_get_path(s, t, path_back_it, comps_V[start_comp], g) == 0)
        {
            Assert(1, std::to_string(s) + "->" + std::to_string(t) + " can not find path in biconnected comp!\n");
        }
    }
    path_back_it = end_cut_point;
}

void get_cut_point_path(int start_comp, VID_TYPE start_v, VID_TYPE t, const std::vector<bctreeNode> &bctree, std::back_insert_iterator<std::vector<VID_TYPE>> cppath_back_it)
{
    if (start_v == t)
    {
        cppath_back_it = t;
        return;
    }
    cppath_back_it = start_v;
    while (bctree[start_comp].parent >= 0)
    {
        cppath_back_it = bctree[start_comp].cut_point;
        start_comp = bctree[start_comp].parent;
    }
    cppath_back_it = t;
}

int remove_2vCut(Graph &g, VID_TYPE s, VID_TYPE t, std::back_insert_iterator<std::vector<VID_TYPE>> path1_back_it, std::back_insert_iterator<std::vector<VID_TYPE>> path2_back_it)
{
    // fake logic!! HAVEN'T IMPLEMENT YET!!
    // ensure path1.size()>=2 && path2.size()>=2 (both must contain s and t)
    std::vector<VID_TYPE> st_nbrs = g.intersect_neighbors(s, t);
    path1_back_it = s;
    if (st_nbrs.size() > 0)
    {
        path1_back_it = st_nbrs[0];
    }
    path1_back_it = t;
    path2_back_it = s;
    if (st_nbrs.size() > 1)
    {
        path2_back_it = st_nbrs[1];
    }
    path2_back_it = t;
    return 1;
}