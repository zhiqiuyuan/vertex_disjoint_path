#include "tools.h"
#include "BoostTools.h"
#include "TwoConnected.h"

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
    std::set<std::pair<int, int>> st_pairs;
    srand(time(0));
    g.generate_rand_vpairs(pairs_cnt, st_pairs);
    int s, t, bs, bt;
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
        std::unordered_map<int, int> new2old;
        bool my_re = st_biconnected_component(g, s, t, new2old);
        std::vector<Edge> after;
        if (my_re)
        {
            int n = g.vertexnum();
            for (int v = 0; v < n; ++v)
            {
                std::vector<int> nbs = g.get_neighbors(v);
                for (int nb : nbs)
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
            std::set<int> boost_V;
            for (auto &e : boost_before)
            {
                boost_V.insert(e.sv);
                boost_V.insert(e.tv);
            }
            for (int v : boost_V)
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
            for (int v = 0; v < n; ++v)
            {
                std::vector<int> nbs = g.get_neighbors(v);
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
//  bin pairs_cnt filename
int main(int argc, char **argv)
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
    std::set<std::pair<int, int>> st_pairs;
    srand(time(0));
    g.generate_rand_vpairs(pairs_cnt, st_pairs);
    int s, t;
    // st_pairs = {{7, 5}}; //{{0, 11}};{{1, 7}};
    for (auto pair : st_pairs)
    {
        s = pair.first;
        t = pair.second;
        print_with_colorln(RED, "s:" + std::to_string(s) + " t:" + std::to_string(t));
        std::vector<int> s_neighbors = g.get_neighbors(s); //= g.delete_vertex(s);
        std::vector<bctreeNode> bctree;                    // idx is comp number too, parent pointer representation
        int t_comp;                                        // if t is not cutpoint: which comp is block(t); if is cutpoint: we set -1
        std::vector<std::set<int>> comps_V;                // comp number->comps vertex set
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
