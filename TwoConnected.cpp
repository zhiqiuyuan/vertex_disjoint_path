#include "TwoConnected.h"

void solve_2VDPP(Graph &g, int s, int t)
{
    std::vector<int> path1, path2;
    std::back_insert_iterator<std::vector<int>> path1_back_it(path1), path2_back_it(path2);
    std::unordered_map<int, int> new2old;
    if (st_biconnected_component(g, s, t, new2old))
    {
        std::vector<int> p1, p2;
        std::back_insert_iterator<std::vector<int>> p1_back_it(p1), p2_back_it(p2);
        if (solve_on_2connected(g, s, t, p1_back_it, p2_back_it))
        {
            // translate path
            map_new2old(0, p1, path1_back_it, new2old);
            map_new2old(0, p2, path2_back_it, new2old);

            print_with_color(BLUE, "path1: ");
            print_vectorln(path1);
            print_with_color(BLUE, "path2: ");
            print_vectorln(path2);
            return;
        }
    }
    print_with_colorln(BLUE, "no solution.");
}

bool solve_on_2connected(Graph &g, int s, int t, std::back_insert_iterator<std::vector<int>> path1_back_it, std::back_insert_iterator<std::vector<int>> path2_back_it)
{
    return remove_2vCut_containing_s(REMOVE_S, g, s, t, path1_back_it, path2_back_it);
}

bool st_biconnected_component(Graph &g, int &s, int &t, std::unordered_map<int, int> &new2old)
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
    // while (p[v] || next_neighbor_idx[s] < s_degree)
    while (p[v] || next_neighbor_idx[s] < s_degree || next_neighbor_idx[v] < g.get_degree(v))
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
                write_graph(g, s, t, new2old, S);
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

int remove_2vCut_containing_s(Remove2VCutSel sel, Graph &g, int s, int t, std::back_insert_iterator<std::vector<int>> path1_back_it, std::back_insert_iterator<std::vector<int>> path2_back_it)
{
    /*remove 2-vertex-cut containing s*/
    std::vector<int> s_neighbors = g.delete_vertex(s);
    std::vector<bctreeNode> bctree;                        // idx is comp number too, parent pointer representation
    int t_comp;                                            // if t is not cutpoint: which comp is block(t); if is cutpoint: we set -1
    std::vector<std::set<int>> comps_V;                    // comp number->comps vertex set
    std::vector<std::vector<int>> s_adj_comp;              // idx correspond to s_neighbors: which comp(s) is s_neighbors[i] in(note that if s_neighbors[i] is cutpoint, more than 1 located comp; else 1 located comp)
    std::unordered_map<int, std::vector<int>> comp2s_nbrs; // if one comp contains one s_neighbors, record it
    bool t_is_cut_point = build_bctree(g, t, s_neighbors, bctree, t_comp, comps_V, s_adj_comp);
    if (comps_V.size() > 1)
    {
        // s_adj_comp->comp2s_nbrs
        for (size_t i = 0; i < s_adj_comp.size(); ++i)
        {
            for (int comp : s_adj_comp[i])
                comp2s_nbrs[comp].push_back(s_neighbors[i]);
        }
        if (t_is_cut_point)
        {
            int compu, compv;
            int u, v;
            no_common_ancestor(compu, compv, u, v, s_adj_comp, bctree);
            std::vector<int> u2t, v2t;
            std::back_insert_iterator<std::vector<int>> u2t_back_it(u2t), v2t_back_it(v2t);
            // t(i.e. root of bctree) is cutpoint,thus t is the destination of the last path segment
            get_parent_path(compu, u, t, u2t_back_it, g, bctree, comps_V);
            get_parent_path(compv, v, t, v2t_back_it, g, bctree, comps_V);
            path1_back_it = s; // Using the assignment operator on the back_insert_iterator (both while being dereferenced or not), causes the container to expand by one element, which is initialized to the value assigned.
            std::copy(u2t.begin(), u2t.end(), path1_back_it);
            path2_back_it = s;
            std::copy(v2t.begin(), v2t.end(), path2_back_it);
            return 1;
        }
        // t is not cut point
        else
        {
            if (comp2s_nbrs.count(t_comp))
            {
                std::vector<int> link_ends = comp2s_nbrs[t_comp];
                if (link_ends.size() >= 2) // u v
                {
                    // new g: b(t), edge(s,u), edge(s,v)
                    std::unordered_map<int, int> new2old;
                    g.recover_vertex(s, {link_ends[0], link_ends[1]});
                    comps_V[t_comp].insert(s);
                    // int old_s = s;
                    write_graph(g, s, t, new2old, comps_V[t_comp]);
                    // comps_V[t_comp].erase(old_s);

                    /*remove 2-vertex-cut containing t*/
                    std::vector<int> p1, p2;
                    std::back_insert_iterator<std::vector<int>> p1_back_it(p1), p2_back_it(p2);
                    int code;
                    if (sel == REMOVE_S)
                        code = remove_2vCut_containing_s(REMOVE_T, g, t, s, p1_back_it, p2_back_it);
                    else
                        code = remove_2vCut_containing_s(REMOVE_S, g, s, t, p1_back_it, p2_back_it);
                    if (code)
                    {
                        // translate path
                        map_new2old(0, p1, path1_back_it, new2old);
                        map_new2old(0, p2, path2_back_it, new2old);
                    }
                    return code;
                }
                else // u
                {
                    // new g: b(t), edge(s,u), edge(s,v)
                    // in which v is: start from any s_adj_comp(except b(t)) start_comp (parent path), the cut point adjancent to t
                    int u = link_ends[0];
                    int start_comp;
                    // must exist: s has only 1 link to b(t), given original G 2connected, there must be another link from s to other comp
                    for (auto p : comp2s_nbrs)
                    {
                        if (p.first != t_comp)
                        {
                            start_comp = p.first;
                            break;
                        }
                    }
                    // s->k-(parent path)->v-(t_comp)->t
                    int k = comp2s_nbrs[start_comp][0];
                    std::vector<int> k2v, k2t_cutpoint;
                    std::back_insert_iterator<std::vector<int>> k2v_back_it(k2v), k2t_cutpoint_back_it(k2t_cutpoint);
                    get_cut_point_path(start_comp, k, t, bctree, k2t_cutpoint_back_it);
                    int v = k2t_cutpoint[k2t_cutpoint.size() - 2]; // last is t, the second last is v (2nd last must exist, otherwise this s_nbr is located in b(t), which contradict with only one link from s to b(t))
                    get_parent_path(start_comp, k, v, k2v_back_it, g, bctree, comps_V);

                    // new g: b(t), edge(s,u), edge(s,v)
                    std::unordered_map<int, int> new2old;
                    g.recover_vertex(s, {u, v});
                    comps_V[t_comp].insert(s);
                    // int old_s = s;
                    write_graph(g, s, t, new2old, comps_V[t_comp]);
                    // comps_V[t_comp].erase(old_s);
                    std::vector<int> p1, p2; // s->u-->t, s->v-->t
                    std::back_insert_iterator<std::vector<int>> p1_back_it(p1), p2_back_it(p2);
                    int code;
                    if (sel == REMOVE_S)
                        code = remove_2vCut_containing_s(REMOVE_T, g, t, s, p1_back_it, p2_back_it);
                    else
                        code = remove_2vCut_containing_s(REMOVE_S, g, s, t, p1_back_it, p2_back_it);
                    if (code)
                    {
                        // assemble and translate path
                        // path1: s->k-->v-->t (p1/2 is s->v-->t)
                        // path2: s->u-->t (p2/1 is s->u-->t)
                        path1_back_it = new2old[s];
                        std::copy(k2v.begin(), k2v.end(), path1_back_it);
                        if (p1[1] == v)
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
                    return code;
                }
            }
            // s no link to b(t)
            else
            {
                // path1:s->k1-(parent path)->a-(t_comp)->t
                // path2:s->k2-(parent path)->b-(t_comp)->t
                // new g: b(t), edge(s,a), edge(s,b)
                int start_comp1, start_comp2;
                int k1, k2;
                no_common_ancestor(start_comp1, start_comp2, k1, k2, s_adj_comp, bctree);

                std::vector<int> k12a, k22b, k12t_cutpoint, k22t_cutpoint;
                std::back_insert_iterator<std::vector<int>> k12a_back_it(k12a), k22b_back_it(k22b), k12t_cutpoint_back_it(k12t_cutpoint), k22t_cutpoint_back_it(k22t_cutpoint);
                get_cut_point_path(start_comp1, k1, t, bctree, k12t_cutpoint_back_it);
                int a = k12t_cutpoint[k12t_cutpoint.size() - 2]; // last is t, the second last is a
                get_parent_path(start_comp1, k1, a, k12a_back_it, g, bctree, comps_V);
                get_cut_point_path(start_comp2, k2, t, bctree, k22t_cutpoint_back_it);
                int b = k22t_cutpoint[k22t_cutpoint.size() - 2]; // last is t, the second last is b
                get_parent_path(start_comp2, k2, b, k22b_back_it, g, bctree, comps_V);

                // new g: b(t), edge(s,a), edge(s,b)
                std::unordered_map<int, int> new2old;
                g.recover_vertex(s, {a, b});
                comps_V[t_comp].insert(s);
                // int old_s = s;
                write_graph(g, s, t, new2old, comps_V[t_comp]);
                // comps_V[t_comp].erase(old_s);
                std::vector<int> p1, p2; // s->a-->t, s->b-->t
                std::back_insert_iterator<std::vector<int>> p1_back_it(p1), p2_back_it(p2);
                int code;
                if (sel == REMOVE_S)
                    code = remove_2vCut_containing_s(REMOVE_T, g, t, s, p1_back_it, p2_back_it);
                else
                    code = remove_2vCut_containing_s(REMOVE_S, g, s, t, p1_back_it, p2_back_it);
                if (code)
                {
                    // assemble and translate path
                    // path1: s->k1-->a-->t (p1/2 is s->a-->t)
                    // path2: s->k2-->b-->t (p2/1 is s->b-->t)
                    path1_back_it = new2old[s];
                    std::copy(k12a.begin(), k12a.end(), path1_back_it);
                    path2_back_it = new2old[s];
                    std::copy(k22b.begin(), k22b.end(), path2_back_it);
                    if (p1[1] == a)
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
                return code;
            }
        }
    }
    // comps_V.size() <= 1: no 2-vertex-cut containing s, recover s neighbors
    else
    {
        g.recover_vertex(s, s_neighbors);
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
            return remove_2vCut(g, s, t, path1_back_it, path2_back_it);
        }
    }
}

#if DEBUG_LEVEL <= TRACE
void print_cutpoint2comp(std::unordered_map<int, std::vector<int>> cutpoint2comp)
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
inline void insert_v_into_comp(std::vector<std::set<int>> &comps_V, std::vector<std::vector<int>> &s_adj_comp, std::vector<bctreeNode> &bctree, int k, int v, std::unordered_map<int, int> &snbrs2idx, std::unordered_map<int, std::vector<int>> cutpoint2comp)
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
inline void insert_parentcutv_into_comp(std::vector<std::set<int>> &comps_V, std::vector<std::vector<int>> &s_adj_comp, int k, int v, std::unordered_map<int, int> &snbrs2idx)
{
    comps_V[k].insert(v);
    // if v is snbr
    // record v's located comp(i.e. comp k) to snbr
    if (snbrs2idx.count(v))
    {
        s_adj_comp[snbrs2idx[v]].push_back(k);
    }
}

bool build_bctree(Graph &g, int t, const std::vector<int> s_neighbors, std::vector<bctreeNode> &bctree, int &t_comp, std::vector<std::set<int>> &comps_V, std::vector<std::vector<int>> &s_adj_comp)
{
    std::unordered_map<int, int> snbrs2idx; // to idx in s_neighbors
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

    int n = g.vertexnum();
    std::vector<int> nr(n, 0);                  // dfs number
    std::vector<int> p(n, 0);                   // dfs parent
    std::vector<int> lowpt(n, 0);               // low point
    std::set<std::pair<int, int>> edge_visited; // mark edge visited
    std::vector<int> next_neighbor_idx(n, 0);   // mark the next neighbor idx to visit for each vertex
    int i = 1;                                  // current dfs number to assign
    int k = 0;                                  // current comp number
    int v = t;                                  // current vertex
    nr[v] = i;
    lowpt[v] = v;
    int w;
    std::unordered_map<int, std::vector<int>> cutpoint2comp; // bctreeNode(.second[i]).cut_point is .first
    // all comps that hanging at each cutpoint. used for determine parent comp for bctree
    std::set<int> t_comps;

    std::vector<int> S = {v}; // stack of vertex
    int t_degree = g.get_degree(t);
    // while (p[v] || next_neighbor_idx[t] < t_degree)
    while (p[v] || next_neighbor_idx[t] < t_degree || next_neighbor_idx[v] < g.get_degree(v))
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
                    int tmpv = S.back();
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
            {
                t_comps.insert(k);
                comps_V.resize(k + 1);
                while (S.back() != v)
                {
                    int tmpv = S.back();
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
void print_comps_V(const std::vector<std::set<int>> &comps_V)
{
    print_with_colorln(BLUE, "comps_V");
    for (size_t i = 0; i < comps_V.size(); ++i)
    {
        std::cout << "comp " << i << ":" << std::endl;
        for (int v : comps_V[i])
        {
            std::cout << v << " ";
        }
        std::cout << std::endl;
    }
}
void print_s_adj_comp(const std::vector<std::vector<int>> &s_adj_comp, const std::vector<int> &s_neighbors)
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

void no_common_ancestor(int &compu, int &compv, int &u, int &v, const std::vector<std::vector<int>> &s_adj_comp, const std::vector<bctreeNode> &bctree)
{
}

void get_parent_path(int start_comp, int start_v, int end_cut_point, std::back_insert_iterator<std::vector<int>> path_back_it, const Graph &g, const std::vector<bctreeNode> &bctree, const std::vector<std::set<int>> &comps_V)
{
}

void get_cut_point_path(int start_comp, int start_v, int t, const std::vector<bctreeNode> &bctree, std::back_insert_iterator<std::vector<int>> cppath_back_it)
{
}

int remove_2vCut(Graph &g, int s, int t, std::back_insert_iterator<std::vector<int>> path1_back_it, std::back_insert_iterator<std::vector<int>> path2_back_it)
{
    return 0;
}