#include "tools.h"
#include "BoostTools.h"
#include "TwoConnected.h"

#if TIME_KILL_ENABLE == 1
bool is_time_out;
#endif //#if TIME_KILL_ENABLE == 1

#if DEBUG_LEVEL <= TRACE
// test st_biconnected_component
//  bin pairs_cnt filename
int main1(int argc, char **argv)
{
    std::ofstream fout("error_log.txt");
    int pairs_cnt = atoi(argv[1]);
    std::string fname = argv[2];
    TVEGraph g;
    if (g.buildGraph(fname) == 0)
    {
        printErrorWithLocation("buildGraph failed!", __FILE__, __LINE__);
        return -1;
    }
    graph_t g_boost;
    to_boost_graph(g_boost, g);
    long long n = g.vertexnum();
    long long pairs_upper = n * (n - 1) / 2;
    if (pairs_cnt > pairs_upper)
    {
        pairs_cnt = pairs_upper;
    }
    std::set<std::pair<VID_TYPE, VID_TYPE>> st_pairs;
    srand(time(0));
    g.generate_rand_vpairs(pairs_cnt, st_pairs);
    VID_TYPE s, t, bs, bt;
    // st_pairs = {{1, 7}}; //{5, 10}
    for (auto pair : st_pairs)
    {
        s = pair.first;
        t = pair.second;
        std::cout << s << "," << t << std::endl;
        bs = s;
        bt = t;
        std::vector<Edge> boost_before, boost_after;
        bool boost_re = st_biconnected_component(g_boost, bs, bt, boost_before, boost_after, 0);
        std::unordered_map<VID_TYPE, VID_TYPE> new2old;
        bool my_re = rec_st_biconnected_component(g, s, t, new2old);
        std::vector<Edge> after;
        if (my_re)
        {
            VID_TYPE n = g.vertexnum();
            for (VID_TYPE v = 0; v < n; ++v)
            {
                std::vector<VID_TYPE> nbs = g.get_neighbors(v);
                for (VID_TYPE nb : nbs)
                {
                    if (v < nb)
                    {
                        after.push_back({v, nb});
                    }
                    else
                    {
                        after.push_back({nb, v});
                    }
                }
            }
            std::sort(after.begin(), after.end());
            auto it = std::unique(after.begin(), after.end());
            after.resize(it - after.begin());
        }

        if (boost_re == my_re)
        {
            if (boost_re)
            {
                if (boost_before == after)
                {
                    continue;
                }
            }
            else
            {
                continue;
            }
        }
        // not same, print diff
        print_with_colorln(RED, "not consist");
        /*
        print_with_colorln(RED, fname);
        std::cout << "[" << s << "," << t << "]" << std::endl;
        print_with_colorln(RED, "boost b(s)==b(t)?" + std::to_string(boost_re));
        if (boost_re)
        {
            print_with_colorln(BLUE, "boost_before vertex:");
            std::set<int> boost_V;
            for (auto &e : boost_before)
            {
                boost_V.insert(e.sv);
                boost_V.insert(e.tv);
            }
            for (int v : boost_V)
            {
                std::cout << v << std::endl;
            }
            print_with_colorln(BLUE, "boost_before edge:");
            for (auto &e : boost_before)
            {
                std::cout << e.str() << std::endl;
            }
        }
        print_with_colorln(RED, "my b(s)==b(t)?" + std::to_string(my_re));
        if (my_re)
        {
            print_with_colorln(BLUE, "my_before vertex:");
            for (int v = 0; v < n; ++v)
            {
                std::vector<int> nbs = g.get_neighbors(v);
                if (nbs.empty() == 0)
                {
                    std::cout << v << std::endl;
                }
            }
            print_with_colorln(BLUE, "my_before edge:");
            for (auto &e : after)
            {
                std::cout << e.str() << std::endl;
            }
        }
        */
        fout << fname << std::endl;
        fout << "[" << s << "," << t << "]" << std::endl;
        fout << "boost b(s)==b(t)?" + std::to_string(boost_re) << std::endl;
        if (boost_re)
        {
            fout << "boost_before vertex:" << std::endl;
            std::set<VID_TYPE> boost_V;
            for (auto &e : boost_before)
            {
                boost_V.insert(e.sv);
                boost_V.insert(e.tv);
            }
            for (VID_TYPE v : boost_V)
            {
                fout << v << std::endl;
            }
            fout << "boost_before edge:" << std::endl;
            for (auto &e : boost_before)
            {
                fout << e.str() << std::endl;
            }
        }
        fout << "my b(s)==b(t)?" + std::to_string(my_re) << std::endl;
        if (my_re)
        {
            fout << "my_before vertex:" << std::endl;
            for (VID_TYPE v = 0; v < n; ++v)
            {
                std::vector<VID_TYPE> nbs = g.get_neighbors(v);
                if (nbs.empty() == 0)
                {
                    fout << v << std::endl;
                }
            }
            fout << "my_before edge:" << std::endl;
            for (auto &e : after)
            {
                fout << e.str() << std::endl;
            }
        }
    }
    fout.close();
    return 0;
}

// test build_bctree
// bin pairs_cnt filename
int main2(int argc, char **argv)
{
    // std::ofstream fout("error_log.txt");
    int pairs_cnt = atoi(argv[1]);
    std::string fname = argv[2];
    TVEGraph g;
    if (g.buildGraph(fname) == 0)
    {
        printErrorWithLocation("buildGraph failed!", __FILE__, __LINE__);
        return -1;
    }
    long long n = g.vertexnum();
    long long pairs_upper = n * (n - 1) / 2;
    if (pairs_cnt > pairs_upper)
    {
        pairs_cnt = pairs_upper;
    }
    std::set<std::pair<VID_TYPE, VID_TYPE>> st_pairs;
    srand(time(0));
    g.generate_rand_vpairs(pairs_cnt, st_pairs);
    VID_TYPE s, t;
    // st_pairs = {{7, 5}}; //{{0, 11}};{{1, 7}};
    for (auto pair : st_pairs)
    {
        s = pair.first;
        t = pair.second;
        print_with_colorln(RED, "s:" + std::to_string(s) + " t:" + std::to_string(t));
        std::vector<VID_TYPE> s_neighbors = g.get_neighbors(s); //= g.delete_vertex(s);
        std::vector<bctreeNode> bctree;                         // idx is comp number too, parent pointer representation
        int t_comp;                                             // if t is not cutpoint: which comp is block(t); if is cutpoint: we set -1
        std::vector<std::set<VID_TYPE>> comps_V;                // comp number->comps vertex set
        std::vector<std::vector<int>> s_adj_comp;
        bool t_is_cut_point = build_bctree(g, t, s_neighbors, bctree, t_comp, comps_V, s_adj_comp);
#if DEBUG_LEVEL <= DEBUG
        print_with_colorln(BLUE, "t_is_cut_point:" + std::to_string(t_is_cut_point));
        print_bctree_vector(bctree);
        print_comps_V(comps_V);
        print_s_adj_comp(s_adj_comp, s_neighbors);
        print_with_colorln(BLUE, "t_comp:" + std::to_string(t_comp));
#endif //#if DEBUG_LEVEL <= DEBUG
        std::cout << std::endl;

        // g.recover_vertex(s, s_neighbors);
    }
    // fout.close();
    return 0;
}

// test get_parent_path
//  bin filename
int main3(int argc, char **argv)
{
    // std::ofstream fout("error_log.txt");
    std::string fname = argv[1];
    TVEGraph g;
    if (g.buildGraph(fname) == 0)
    {
        printErrorWithLocation("buildGraph failed!", __FILE__, __LINE__);
        return -1;
    }
    std::vector<VID_TYPE> ts = {1, 7, 0};
    std::vector<VID_TYPE> path;
    std::back_insert_iterator<std::vector<VID_TYPE>> path_back_it(path);
    for (auto t : ts)
    {
        print_with_colorln(RED, " t:" + std::to_string(t));
        std::vector<VID_TYPE> s_neighbors;
        std::vector<bctreeNode> bctree;          // idx is comp number too, parent pointer representation
        int t_comp;                              // if t is not cutpoint: which comp is block(t); if is cutpoint: we set -1
        std::vector<std::set<VID_TYPE>> comps_V; // comp number->comps vertex set
        std::vector<std::vector<int>> s_adj_comp;
        bool t_is_cut_point = build_bctree(g, t, s_neighbors, bctree, t_comp, comps_V, s_adj_comp);
        print_with_colorln(BLUE, "t_is_cut_point:" + std::to_string(t_is_cut_point));
        print_bctree_vector(bctree);
        print_comps_V(comps_V);
        for (size_t i = 0; i < comps_V.size(); ++i)
        {
            for (VID_TYPE startv : comps_V[i])
            {
                std::cout << "start_comp(" << i << ")startv:" << startv << "->" << t << ":" << std::endl;
                path.clear();
                get_parent_path(i, startv, t, path_back_it, g, bctree, comps_V);
                print_vectorln(path);
            }
        }
        std::cout << std::endl;
        std::cout << std::endl;

        // g.recover_vertex(s, s_neighbors);
    }
    // fout.close();
    return 0;
}

// test no_common_ancestor get_cut_point_path
// bin
int main4(int argc, char **argv)
{
    std::vector<bctreeNode> bctree = {
        // parent cutpoint
        {3, 0},
        {2, 1},
        {3, 2},
        {10, 3},
        {2, 4},
        {10, 5},
        {5, 6},
        {5, 7},
        {6, 8},
        {7, 9},
        {-1, 10}};
    std::vector<int> leaves = {8, 9, 0, 1, 4};
    VID_TYPE n = 11;
    int t_comp = 10;
    print_with_colorln(BLUE, "comp:\tancestor");
    for (VID_TYPE i = 0; i < n; ++i)
    {
        std::cout << i << "\t" << get_root_comp(i, bctree, t_comp) << std::endl;
    }
    int compu, compv;
    VID_TYPE u, v;
    std::vector<std::vector<int>> s_adj_comp = {
        {8, 1},
        {9},
        {4}};
    /*
    {8},
        {9},
        {1},
        {4}};
    */
    /*no_common_ancestor(compu, compv, u, v, s_adj_comp, bctree, t_comp);
    std::cout << "comp\tidx\n"
              << compu << "\t" << u << "\n"
              << compv << "\t" << v << std::endl;*/
    VID_TYPE t = 10;
    std::vector<VID_TYPE> cppath;
    std::back_insert_iterator<std::vector<VID_TYPE>> cppath_back_it(cppath);
    for (int leafcomp : leaves)
    {
        cppath.clear();
        std::cout << "leafcomp:" << leafcomp << std::endl;
        get_cut_point_path(leafcomp, leafcomp, t, bctree, cppath_back_it);
        for (size_t i = 1; i < cppath.size(); ++i)
        {
            std::cout << cppath[i] << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}

#endif //#if DEBUG_LEVEL <= TRACE

// standard biconnected_component_decompose
// bin filename
// 5
int main5(int argc, char **argv)
{
    std::string fname = argv[1];
    std::cout << fname << std::endl;
    // TVEGraph g;
    EGraph g, g_copy;
    if (g.buildGraph(fname) == 0)
    {
        printErrorWithLocation("buildGraph failed!", __FILE__, __LINE__);
        return -1;
    }
    std::vector<VID_TYPE> s_neighbors = g.delete_vertex(3);

    g_copy = g;
    VID_TYPE s = 0;
    VID_TYPE t = 4;
    std::unordered_map<VID_TYPE, VID_TYPE> new2old;
    /* my root:s*/
    // st_biconnected_component(g, s, t, new2old);

    /* rec function call, stack overflow when tesing on big graph root:s*/
    ///*
    new2old.clear();
    g = g_copy;
    rec_st_biconnected_component(g, s, t, new2old);
    //*/

    /* boost std root:s*/
    /*
    g = g_copy;
    graph_t bg;
    to_boost_graph(bg, g);
    std::vector<Edge> before, after;
    st_biconnected_component(bg, s, t, before, after, 1); // s is root
    */

    /* my build_bctree root:t*/
    ///*
    std::vector<bctreeNode> bctree;          // idx is comp number too, parent pointer representation
    int t_comp;                              // if t is not cutpoint: which comp is block(t); if is cutpoint: we set -1
    std::vector<std::set<VID_TYPE>> comps_V; // comp number->comps vertex set
    std::vector<std::vector<int>> s_adj_comp;
    t = s;
    bool t_is_cut_point = build_bctree(g, t, s_neighbors, bctree, t_comp, comps_V, s_adj_comp);
#if DEBUG_LEVEL <= DEBUG
    print_with_colorln(BLUE, "t_is_cut_point:" + std::to_string(t_is_cut_point));
    print_bctree_vector(bctree);
    print_comps_V(comps_V);
    // print_s_adj_comp(s_adj_comp, s_neighbors);
    print_with_colorln(BLUE, "t_comp:" + std::to_string(t_comp));
#endif //#if DEBUG_LEVEL <= DEBUG
       //*/
    return 0;
}

// test solve_2VDPP
// bin pairs_cnt filename
// 6
int main(int argc, char **argv)
{
    // std::ofstream fout("error_log.txt");
    int pairs_cnt = atoi(argv[1]);
    std::string fname = argv[2];
    TVEGraph g, g_copy;
    // EGraph g, g_copy;
    if (g.buildGraph(fname) == 0)
    {
        printErrorWithLocation("buildGraph failed!", __FILE__, __LINE__);
        return -1;
    }
    g_copy = g;
    std::set<std::pair<VID_TYPE, VID_TYPE>> st_pairs;
    /*
    for (int i = 0; i < 10; ++i)
    {
        for (int j = i + 1; j < 10; ++j)
        {
            st_pairs.insert({i, j});
        }
    }
    */
    VID_TYPE s, t;
    st_pairs = {{107, 263249}};
    for (auto pair : st_pairs)
    {
        s = pair.first;
        t = pair.second;
        print_with_colorln(RED, "s:" + std::to_string(s) + " t:" + std::to_string(t));
        solve_2VDPP(g, s, t);
        std::cout << std::endl;
        g = g_copy;
    }
    // fout.close();
    return 0;
}
