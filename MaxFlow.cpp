#include "MaxFlow.h"

#if TIME_KILL_ENABLE == 1
extern bool is_time_out;
#endif //#if TIME_KILL_ENABLE == 1

int maxflow(MemGraph &g, VID_TYPE s, VID_TYPE t)
{
    std::vector<std::vector<VID_TYPE>> paths;
    int code = maxflow_based(g, s, t, 2, paths);
    if (code == 1)
    {
        if (paths.size() < 2)
        {
            print_with_colorln(RED, "reported found 2 but returned paths are less than 2!");
            return 0;
        }
        print_with_color(BLUE, "path1: ");
        print_vectorln(paths[0]);
        print_with_color(BLUE, "path2: ");
        print_vectorln(paths[1]);
        return code;
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

int maxflow_based(MemGraph &g, VID_TYPE s, VID_TYPE t, VID_TYPE disjoint_path_num, std::vector<std::vector<VID_TYPE>> &disjoint_paths)
{
    if (disjoint_path_num < 1)
    {
        return 1;
    }
    SplitedGraphWrapper Gwrapper(std::move(g), s, t);
#if DEBUG_LEVEL <= TRACE
    //(Gwrapper.G)->print_graph();
    //(Gwrapper.G)->print_graph_plot_format();
#endif //#if DEBUG_LEVEL <= TRACE
    std::vector<std::vector<VID_TYPE>> pre_disjoint_paths(disjoint_path_num);
    VID_TYPE path_cnt = 0;
    while (path_cnt < disjoint_path_num)
    {
        std::unordered_map<VID_TYPE, VID_TYPE> vid2label;
        int code = (Gwrapper.G)->dinic_label(vid2label);
#if TIME_KILL_ENABLE == 1
        if (code == TIME_EXCEED_RESULT)
        {
            return TIME_EXCEED_RESULT;
        }
#endif //#if TIME_KILL_ENABLE==1
#if DEBUG_LEVEL <= TRACE
        print_umapln(vid2label);
#endif //#if DEBUG_LEVEL <= TRACE

        // label can't reach t, end of dinic
        if (code == 0)
        {
            break;
        }

        while (path_cnt < disjoint_path_num)
        {
            code = (Gwrapper.G)->dinic_trace_path(vid2label, pre_disjoint_paths[path_cnt]);
#if DEBUG_LEVEL <= TRACE
            print_vectorln(pre_disjoint_paths[path_cnt]);
#endif //#if DEBUG_LEVEL <= TRACE
#if TIME_KILL_ENABLE == 1
            if (code == TIME_EXCEED_RESULT)
            {
                return TIME_EXCEED_RESULT;
            }
#endif //#if TIME_KILL_ENABLE==1
            if (code == 0)
            {
                break;
            }
            ++path_cnt;
        }
    }

    // construct path in ori_graph
    if (path_cnt == disjoint_path_num)
    {
        return (Gwrapper.G)->dinic_modify_path(pre_disjoint_paths, disjoint_paths);
    }
    return 0;
}

int SplitedGraph::dinic_label(std::unordered_map<VID_TYPE, VID_TYPE> &vid2label)
{
    std::queue<VID_TYPE> q, next_q;
    q.push(s);
    // labeling when pushing into queue
    vid2label[s] = 0;
    bool reached_t = 0;
    VID_TYPE v, label;
    while (q.empty() == 0)
    {
        // scan one level
        while (q.empty() == 0)
        {
#if TIME_KILL_ENABLE == 1
            if (is_time_out)
                return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
            v = q.front();
            q.pop();
            // scanning v
            label = vid2label[v] + 1;
            for (VID_TYPE u : get_out_neighbors(v))
            {
#if TIME_KILL_ENABLE == 1
                if (is_time_out)
                    return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
                if (vid2label.count(u) == 0)
                {
                    vid2label[u] = label;
                    if (u == t)
                        reached_t = 1;
                    if (reached_t == 0) // if have met t, no need to push into queue anymore
                        next_q.push(u);
                }
            }
        }
        if (reached_t) // reached t while scanning this level, and this level has all been scanned
        {
            break;
        }
        q = std::move(next_q);
    }
    return reached_t;
}

int SplitedGraph::dinic_trace_path(const std::unordered_map<VID_TYPE, VID_TYPE> &vid2label, std::vector<VID_TYPE> &path)
{
    std::vector<VID_TYPE> curr_path;
    std::stack<VID_TYPE> stk;
    stk.push(s);
    curr_path.push_back(s); // add to path when pushing into stack(because test t when pushing into stack)
    std::unordered_map<VID_TYPE, VID_TYPE> next_nbr_pos;
    next_nbr_pos[s] = 0;
    VID_TYPE v, u;
    // s cannot be t, so test t when pushing into stack
    while (stk.empty() == 0)
    {
#if DEBUG_LEVEL <= TRACE
        print_stackln(stk);
#endif //#if DEBUG_LEVEL <= TRACE
#if TIME_KILL_ENABLE == 1
        if (is_time_out)
            return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
        v = stk.top();
        if (next_nbr_pos[v] == get_out_degree(v))
        {
            stk.pop();
            curr_path.pop_back();
            continue;
        }
        const std::vector<VID_TYPE> &v_nbrs = get_out_neighbors(v);
        VID_TYPE v_deg = v_nbrs.size(), label = vid2label.at(v) + 1;
        VID_TYPE i;
        for (i = next_nbr_pos[v]; i < v_deg; ++i)
        {
#if TIME_KILL_ENABLE == 1
            if (is_time_out)
                return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
            if (vid2label.count(v_nbrs[i]) && vid2label.at(v_nbrs[i]) == label)
            {
                u = v_nbrs[i];
                curr_path.push_back(u);
                if (u == t)
                {
                    path = std::move(curr_path);
                    // reverse path in G
                    reverse_path(path);
                    return 1;
                }
                stk.push(u);
                next_nbr_pos[u] = 0;
                ++i;
                break;
            }
        }
        next_nbr_pos[v] = i;
    }
    // can not reach t
    return 0;
}

void SplitedGraph::reverse_path(const std::vector<VID_TYPE> &path)
{
    if (path.size() <= 1)
    {
        return;
    }
    VID_TYPE A = -1;
    auto C_it = path.begin();
    ++C_it;
    for (VID_TYPE B : path)
    {
        // B is t
        if (C_it == path.end())
        {
            swap_neighbors(B, -1, A);
            break;
        }
        swap_neighbors(B, *C_it, A);
        A = B;
        ++C_it;
    }
}

int SplitedGraph::dinic_modify_path(const std::vector<std::vector<VID_TYPE>> &pre_disjoint_paths, std::vector<std::vector<VID_TYPE>> &disjoint_paths)
{
    if (pre_disjoint_paths.size() < 1)
    {
        return 1;
    }

    // construct a new graph: edges are all edges in pre_disjoint_paths (all are edge disjoint), after removing dup edges
    // edge disjoint: except s and t, all vertex has exactly one out_nbr; s has many, t has no
    std::unordered_map<VID_TYPE, VID_TYPE> out_nbr;
    std::unordered_set<VID_TYPE> s_nbrs;
    VID_TYPE p_sz, u, v;
    for (const auto &pre_p : pre_disjoint_paths)
    {
#if TIME_KILL_ENABLE == 1
        if (is_time_out)
            return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
        p_sz = pre_p.size();
        for (VID_TYPE i = 1; i < p_sz; ++i)
        {
            // pre_p[i-1]->pre_p[i]
            // u->v
            u = pre_p[i - 1];
            v = pre_p[i];
            if (u == s)
            {
                if (s_nbrs.count(v))
                {
                    print_with_colorln(RED, "not edge disjoint! s" + std::to_string(u) + " already has out_nbr" + std::to_string(v));
                    return 0;
                }
                s_nbrs.insert(v);
                continue;
            }
            // if v->u exists: remove v-u
            if (out_nbr.count(v) && out_nbr.at(v) == u)
            {
                out_nbr.erase(v);
            }
            else if (out_nbr.count(u))
            {
                print_with_colorln(RED, "not edge disjoint! " + std::to_string(u) + " already has out_nbr" + std::to_string(out_nbr[u]));
                return 0;
            }
            else
            {
                out_nbr[u] = v;
            }
        }
    }
#if DEBUG_LEVEL <= TRACE
    print_umapln(out_nbr);
    print_usetln(s_nbrs, s);
#endif //#if DEBUG_LEVEL <= TRACE

    std::vector<VID_TYPE> modified_path;
    // find all paths from s to t and translate: edge disjoint, so each step in each path has exactly 1 branch
    VID_TYPE disjoint_path_num = pre_disjoint_paths.size();
    disjoint_paths.resize(disjoint_path_num);
    VID_TYPE i = 0;
    for (VID_TYPE s_nbr : s_nbrs)
    {
#if TIME_KILL_ENABLE == 1
        if (is_time_out)
            return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
        modified_path.push_back(s);
        VID_TYPE v = s_nbr;
        while (out_nbr.count(v))
        {
            modified_path.push_back(v);
            v = out_nbr[v];
        }
        // t has no out_nbr
        if (v != t)
        {
            print_with_colorln(RED, "some path fail to reach t" + std::to_string(t));
            return 0;
        }
        modified_path.push_back(t);

#if DEBUG_LEVEL <= TRACE
        print_vectorln(modified_path);
#endif //#if DEBUG_LEVEL <= TRACE

        // translate
        if (translate_path(std::move(modified_path), disjoint_paths[i]) == 0)
        {
            return 0;
        }
#if DEBUG_LEVEL <= TRACE
        print_vectorln(disjoint_paths[i]);
#endif //#if DEBUG_LEVEL <= TRACE
        ++i;
    }
    return 1;
}

int SplitedGraph::translate_path(std::vector<VID_TYPE> modified_path, std::vector<VID_TYPE> &path)
{
    VID_TYPE sz = modified_path.size();
    if (sz < 1)
    {
        return 1;
    }
    if (sz % 2 == 0)
    {
        path.push_back(s - ori_vertex_num);
        VID_TYPE i = 1;
        --sz;
        while (i < sz)
        {
            if (modified_path[i] + ori_vertex_num != modified_path[i + 1])
            {
                print_with_colorln(RED, "wrong vertex sequence format: 'a a+n' is violated");
                return 0;
            }
            path.push_back(modified_path[i]);
            i += 2;
        }
        path.push_back(t);
        return 1;
    }
    print_with_colorln(RED, "wrong vertex sequence format: modified_path is odd sz");
    return 0;
}