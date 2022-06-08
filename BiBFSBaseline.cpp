#include "BiBFSBaseline.h"
#if TIME_KILL_ENABLE == 1
extern bool is_time_out;
#endif //#if TIME_KILL_ENABLE == 1

void BiBFSBaseline::clear_all()
{
    path1_vSet.clear();
    path1_vSeq.clear();
    path2_vSeq.clear();
    path1_joint = 0;
}

void BiBFSBaseline::clear_path2()
{
    path2_vSeq.clear();
}

BiBFSBaseline::BiBFSBaseline()
{
    clear_all();
}

BiBFSBaseline::~BiBFSBaseline()
{
    for (auto p : src_v2visited)
    {
        delete p.second;
    }
    for (auto p : tat_v2visited)
    {
        delete p.second;
    }
}

int BiBFSBaseline::bibfs(Graph &g, long long side_depth, int s, int t)
{
    src_side_max_depth = bfs(side_depth, g, s, src_v2visited);
    if (src_side_max_depth == TIME_EXCEED_RESULT)
    {
        return TIME_EXCEED_RESULT;
    }
    max_depth_sum = bfs(side_depth, g, t, tat_v2visited);
    if (max_depth_sum == TIME_EXCEED_RESULT)
    {
        return TIME_EXCEED_RESULT;
    }
    max_depth_sum += src_side_max_depth;
    sv = s;
    tv = t;
    path1_vSet.insert(sv);
    path1_vSet.insert(tv);
    G = &g;
    return 1;
}

int BiBFSBaseline::src_side_dfs(VisitedNode *start)
{
#if TIME_KILL_ENABLE == 1
    if (is_time_out)
        return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
    int startv = start->v;

    //走到s了：搜索joint的tat_side(s不加入path1record)
    if (startv == sv)
    {
#if DEBUG_LEVEL <= TRACE
        print_with_colorln(DARK_YELLOW, "\tpath1 to t:");
#endif //#if DEBUG_LEVEL <= TRACE

        //把joint从path1_vSeq和path1_vSet中删除，tat_side_dfs时加入
        if (path1_joint != sv) // path1_joint == sv则joint没有加入path1_vSeq，不用从path1_vSeq中删除
        {
            path1_vSet.erase(path1_joint);
            path1_vSeq.pop_back();
        }
        else
        {
            path1_vSet.erase(path1_joint);
        }
        int result = tat_side_dfs(tat_v2visited[path1_joint]);
        // !(result == 1 || TIME_EXCEED_RESULT || DEPTH_LIMIT_EXCEED_RESULT)
        if (result == 0)
        {
#if DEBUG_LEVEL <= TRACE
            print_with_colorln(DARK_YELLOW, "\tfor this path1 to s, all path1 to t fail, try another path1 to s:");
#endif //#if DEBUG_LEVEL <= TRACE

            //把joint加回去给path1_vSeq和path1_vSet
            if (path1_joint != sv)
            {
                path1_vSet.insert(path1_joint);
                path1_vSeq.push_back(path1_joint);
            }
            else
            {
                path1_vSet.insert(path1_joint);
            }
            return 0;
        }
        else
        {
            return result;
        }
    }

    if (path1_vSet.count(startv)) // start already in this path
    {
        return 0;
    }

    path1_vSeq.push_front(startv);
#if DEPTH_KILL_ENABLE == 1
    if ((int)path1_vSeq.size() > src_side_max_depth)
    {
        path1_vSeq.pop_front();
        return DEPTH_LIMIT_EXCEED_RESULT;
    }
#endif // DEPTH_KILL_ENABLE==1

    path1_vSet.insert(startv); //标记start在当前路径上
#if DEBUG_LEVEL <= TRACE
    std::cout << "\tpath1:";
    print_pathln(path1_vSeq, 1);
#endif //#if DEBUG_LEVEL <= TRACE

    //继续向s深搜
    size_t sz = start->previous_visited_inBFS.size();
#if DEPTH_KILL_ENABLE == 1
    bool some_exceed_depth_limit = 0;
#endif //#if DEPTH_KILL_ENABLE == 1
    for (size_t i = 0; i < sz; i++)
    {
        int result = src_side_dfs(start->previous_visited_inBFS[i]);
        // 找到了两条独立路径或超时则返回，未找到或者超深度则继续其他孩子
        if (result == 1)
        {
            return result;
        }

#if TIME_KILL_ENABLE == 1
        if (is_time_out)
            return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
#if DEPTH_KILL_ENABLE == 1
        if (result == DEPTH_LIMIT_EXCEED_RESULT)
        {
            some_exceed_depth_limit = 1;
        }
#endif //#if DEPTH_KILL_ENABLE == 1
    }

    path1_vSet.erase(startv);
    path1_vSeq.pop_front();
// if some child exceed depth limit
#if DEPTH_KILL_ENABLE == 1
    if (some_exceed_depth_limit)
    {
        return DEPTH_LIMIT_EXCEED_RESULT;
    }
#endif //#if DEPTH_KILL_ENABLE == 1
    return 0;
}

int BiBFSBaseline::tat_side_dfs(VisitedNode *start)
{
#if TIME_KILL_ENABLE == 1
    if (is_time_out)
        return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
    int startv = start->v;

    //走到t了：找到了一条path1(t不加入path1record)，为path1找path2
    if (startv == tv)
    {
        clear_path2();

        if (find_path2() == 1)
        {
            return 1;
        }
//如果没有找到，要尝试新的path1（选择下一条path1（其实是修改t前面的那一个顶点）由上一层调用控制）
//由于start没有加入path1record，因此无需erase和pop_back
#if DEBUG_LEVEL <= DEBUG
        print_with_colorln(DARK_YELLOW, "find path2 failed, try another path1");
#endif //#if DEBUG_LEVEL <= DEBUG
#if TIME_KILL_ENABLE == 1
        if (is_time_out)
            return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
        return 0;
    }

    // joint!=sv or joint==sv: joint won't be add to path1_vSet in src_side_dfs(in fact is removed)
    if (path1_vSet.count(startv)) // start already in this path
    {
        return 0;
    }

    if (startv != sv)
    {
        path1_vSeq.push_back(startv);
#if DEPTH_KILL_ENABLE == 1
        if ((int)path1_vSeq.size() > max_depth_sum)
        {
            path1_vSeq.pop_back();
            return DEPTH_LIMIT_EXCEED_RESULT;
        }
#endif // DEPTH_KILL_ENABLE==1
    }
    path1_vSet.insert(startv); //标记start在当前路径上 //如果joint==sv，则首次调用不会被卡在if (path1_vSet.count(startv))判断处，另外会从这里在首次调用时被加入vset
#if DEBUG_LEVEL <= TRACE
    std::cout << "\tpath1:";
    print_pathln(path1_vSeq, 1);
#endif //#if DEBUG_LEVEL <= TRACE

    //继续向t深搜
    size_t sz = start->previous_visited_inBFS.size();
#if DEPTH_KILL_ENABLE == 1
    bool some_exceed_depth_limit = 0;
#endif //#if DEPTH_KILL_ENABLE == 1
    for (size_t i = 0; i < sz; i++)
    {
        int result = tat_side_dfs(start->previous_visited_inBFS[i]);
        // 找到了两条独立路径或超时则返回，未找到或者超深度则继续其他孩子
        if (result == 1)
        {
            return result;
        }

#if TIME_KILL_ENABLE == 1
        if (is_time_out)
            return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
#if DEPTH_KILL_ENABLE == 1
        if (result == DEPTH_LIMIT_EXCEED_RESULT)
        {
            some_exceed_depth_limit = 1;
        }
#endif //#if DEPTH_KILL_ENABLE == 1
    }

    path1_vSet.erase(startv);
    if (startv != sv)
    {
        path1_vSeq.pop_back();
    }
// if some child exceed depth limit
#if DEPTH_KILL_ENABLE == 1
    if (some_exceed_depth_limit)
    {
        return DEPTH_LIMIT_EXCEED_RESULT;
    }
#endif //#if DEPTH_KILL_ENABLE == 1
    return 0;
}

int BiBFSBaseline::find_path2()
{
#if DEBUG_LEVEL <= DEBUG
    print_with_colorln(DARK_YELLOW, "path1 done, search path2 for it");
#endif //#if DEBUG_LEVEL <= DEBUG
#if DEBUG_LEVEL <= TRACE
    print_with_color(BLUE, "path1:");
    print_pathln(path1_vSeq, 1);
#endif //#if DEBUG_LEVEL <= TRACE
    /*bibfs from s and t*/
    std::set<int> s_visited = {sv}; // mark as visited; mark when pushing into queue
    std::set<int> t_visited = {tv};
    std::queue<int> sq, tq;
    sq.push(sv);
    tq.push(tv);
    int curr;
    int joint = 0;
    // s->joint: only one prec
    // joint->t: only one succ
    std::map<int, int> prec, succ;
    while (sq.empty() == 0 && tq.empty() == 0 && joint == 0)
    {
#if TIME_KILL_ENABLE == 1
        if (is_time_out)
            return TIME_EXCEED_RESULT;
#endif //#if TIME_KILL_ENABLE == 1
#if DEBUG_LEVEL <= TRACE
        std::cout << "sq:";
        print_queueln(sq);
        std::cout << "tq:";
        print_queueln(tq);
#endif //#if DEBUG_LEVEL <= TRACE
       //选择更小规模的进行扩展 // s->
        if (sq.size() < tq.size())
        {
            curr = sq.front();
            sq.pop();
            std::vector<int> neighbors;
            neighbors = G->get_neighbors(curr);
#if DEBUG_LEVEL <= TRACE
            std::cout << "neighbors:";
            print_vectorln(neighbors);
#endif //#if DEBUG_LEVEL <= TRACE
            for (int n : neighbors)
            {
                if (path1_vSeq.empty() && curr == sv && n == tv)
                { // path1是s->t这样的路径，path2要避免直接s->t
                    continue;
                }
                // 在t_side被访问过
                if (t_visited.count(n))
                {
                    prec[n] = curr;
                    joint = n;
                    break;
                }
                // path1中没有且s_side没有访问过
                if (path1_vSet.count(n) == 0 && s_visited.count(n) == 0)
                {
                    sq.push(n);
                    prec[n] = curr;
                    s_visited.insert(n);
                }
            }
        }
        // t<-
        else
        {
            curr = tq.front();
            tq.pop();
            std::vector<int> neighbors;
            neighbors = G->get_neighbors(curr);
#if DEBUG_LEVEL <= TRACE
            std::cout << "neighbors:";
            print_vectorln(neighbors);
#endif //#if DEBUG_LEVEL <= TRACE
            for (int n : neighbors)
            {
                if (path1_vSeq.empty() && curr == tv && n == sv)
                { // path1是s->t这样的路径，path2要避免直接s->t
                    continue;
                }
                // 在s_side被访问过
                if (s_visited.count(n))
                {
                    succ[n] = curr;
                    joint = n;
                    break;
                }
                // path1中没有且t_side没有访问过
                if (path1_vSet.count(n) == 0 && t_visited.count(n) == 0)
                {
                    tq.push(n);
                    succ[n] = curr;
                    t_visited.insert(n);
                }
            }
        }
    }
    if (joint)
    {
        /*trace path*/
        // tv and sv not included in path2_seq
        curr = joint;
        if (curr != sv && curr != tv)
        {
            path2_vSeq.push_front(curr);
        }
        // s->joint: only one prec
        // need testing prec.count(curr), for maybe joint has no prec(i.e. sv)
        while (prec.count(curr) && prec[curr] != sv)
        {
            curr = prec[curr];
            path2_vSeq.push_front(curr);
        }
        // joint->t: only one succ
        curr = joint;
        // need testing succ.count(curr), for maybe joint has no succ(i.e. tv)
        while (succ.count(curr) && succ[curr] != tv)
        {
            curr = succ[curr];
            path2_vSeq.push_back(curr);
        }
        return 1;
    }
    return 0;
}

int BiBFSBaseline::run()
{
    std::map<int, VisitedNode *>::iterator s_iterator = src_v2visited.begin();
    std::map<int, VisitedNode *>::iterator t_iterator = tat_v2visited.begin();
    while (s_iterator != src_v2visited.end() && t_iterator != tat_v2visited.end())
    {
        int v1 = s_iterator->first;
        int v2 = t_iterator->first;
        if (v1 == v2)
        {
            //一个重合点joint
            path1_joint = v1;
#if DEBUG_LEVEL <= TRACE
            print_with_color(DARK_YELLOW, "path1 joint:" + std::to_string(v1));
            print_with_colorln(DARK_YELLOW, "\n\tpath1 from s:");
#endif //#if DEBUG_LEVEL <= TRACE
            int result = src_side_dfs(src_v2visited[path1_joint]);
            // result == 1 || TIME_EXCEED_RESULT || DEPTH_LIMIT_EXCEED_RESULT
            if (result)
            {
                //这个joint延申出的某条路径可以找到一条与之独立的路径，已经保存到this的member中，或者超时，则可以直接结束
                return result;
            }
            //如果找不到，回溯上来的过程必然已经将path1record清空

            s_iterator++;
            t_iterator++;
        }
        else if (v1 < v2)
        {
            s_iterator++;
        }
        else if (v1 > v2)
        {
            t_iterator++;
        }
    }
    //已经尝试所有路径，每条都找不到一条与之独立的路径
    return 0;
}

void BiBFSBaseline::print_pathln(std::deque<int> path_vSeq, bool print_st)
{
    if (print_st)
    {
        std::cout << sv << " ";
    }
    int sz = path_vSeq.size();
    for (int i = 0; i < sz; ++i)
    {
        std::cout << path_vSeq[i] << " ";
    }
    if (print_st)
    {
        std::cout << tv;
    }
    std::cout << std::endl;
}

void BiBFSBaseline::print_path1path2(bool print_st)
{
    print_with_color(BLUE, "path1:\n");
    print_pathln(path1_vSeq, print_st);
    print_with_color(BLUE, "path2:\n");
    print_pathln(path2_vSeq, print_st);
}

#if DEBUG_LEVEL <= DEBUG
void BiBFSBaseline::print_1l_pvinBFS()
{
    print_with_color(BLUE, "src_v2visited:");
    for (auto item : src_v2visited)
    {
        std::cout << item.first << "\n\t";
        item.second->print_1l_pvinBFS();
        std::cout << "\n";
    }
    print_with_color(BLUE, "tat_v2visited:");
    for (auto item : tat_v2visited)
    {
        std::cout << item.first << "\n\t";
        item.second->print_1l_pvinBFS();
        std::cout << "\n";
    }
}

void BiBFSBaseline::print_jointln()
{
    print_with_colorln(YELLOW, "joints:");
    std::vector<VertexWithNum> joints;
    std::map<int, VisitedNode *>::iterator s_iterator = src_v2visited.begin();
    std::map<int, VisitedNode *>::iterator t_iterator = tat_v2visited.begin();
    while (s_iterator != src_v2visited.end() && t_iterator != tat_v2visited.end())
    {
        int v1 = s_iterator->first;
        int v2 = t_iterator->first;
        if (v1 == v2)
        {
            std::cout << v1 << "\t";
            joints.push_back({v1, (int)src_v2visited[v1]->previous_visited_inBFS.size(), (int)tat_v2visited[v1]->previous_visited_inBFS.size()});

            s_iterator++;
            t_iterator++;
        }
        else if (v1 < v2)
        {
            s_iterator++;
        }
        else if (v1 > v2)
        {
            t_iterator++;
        }
    }
    std::cout << std::endl;
    std::sort(joints.begin(), joints.end());
    // top 2
    print_with_colorln(YELLOW, "\ttop joints(max bfs_parent sum; max bfs_parent num_diff):");
    if (joints.size() > 0)
    {
        std::cout << joints[0].v << "\t";
        if (joints.size() > 1)
        {
            std::cout << joints[1].v << "\t";
        }
        std::cout << std::endl;
    }
}
#endif //#if DEBUG_LEVEL <= DEBUG

void print_1v_1l_pvinBFS(BiBFSBaseline &act, int v)
{
    std::cout << "v" << v;
    std::cout << "\nsrc_v2visited:" << std::endl;
    if (act.src_v2visited.count(v))
    {
        (act.src_v2visited[v])->print_1l_pvinBFS();
    }
    std::cout << std::endl;
    std::cout << "tat_v2visited:" << std::endl;
    if (act.tat_v2visited.count(v))
    {
        (act.tat_v2visited[v])->print_1l_pvinBFS();
    }
    std::cout << std::endl;
}

int baseline(Graph &g, long long side_depth, int s, int t, bool print_result)
{
    BiBFSBaseline act;
    /* step1: 双搜bfs，搜索深度为side_depth */
    print_with_colorln(GREEN, "\t\tstep1: bibfs...");
    if (act.bibfs(g, side_depth, s, t) == TIME_EXCEED_RESULT)
    {
        print_with_colorln(BLUE, "time limit exceed.");
        return TIME_EXCEED_RESULT;
    }
#if DEBUG_LEVEL <= DEBUG
    // act.print_1l_pvinBFS(1);
    act.print_jointln();
    // print_1v_1l_pvinBFS(act, act.sv);
    // print_1v_1l_pvinBFS(act, act.tv);
#endif // #if DEBUG_LEVEL <= DEBUG
    if (side_depth == LLONG_MAX)
    {
        // s搜穿也走不到t，不存在路径从s到t
        if (act.src_v2visited.count(act.tv) == 0)
        {
            if (print_result)
            {
                print_with_colorln(BLUE, "no solution.");
            }
            return 0;
        }
    }
    // s和t度数都必须>=2才可能
    int s_out_degree, t_in_degree;
    s_out_degree = g.get_degree(act.sv);
    t_in_degree = g.get_degree(act.tv);
    if (t_in_degree < 2 || s_out_degree < 2)
    {
        if (print_result)
        {
            print_with_colorln(BLUE, "no solution.");
        }
        return 0;
    }

    /* step2: 找两条独立路径 */
    print_with_colorln(GREEN, "\t\tstep2: search circle...");
    int result = act.run();

    if (print_result)
    {
        if (result == 1)
        {
            // act.path1_vSeq, act.path2_vSeq are what we want
            act.print_path1path2(1);
        }
        else if (result == 0)
        {
            print_with_colorln(BLUE, "no solution.");
        }
#if TIME_KILL_ENABLE == 1
        else if (result == TIME_EXCEED_RESULT)
        {
            print_with_colorln(BLUE, "time limit exceed.");
        }
#endif //#if TIME_KILL_ENABLE == 1
#if DEPTH_KILL_ENABLE == 1
        else if (result == DEPTH_LIMIT_EXCEED_RESULT)
        {
            print_with_color(BLUE, "Depth limit exceed for some and failed for other!");
            std::cout << " src_side_max_depth:" << act.src_side_max_depth << " ";
            std::cout << " tat_side_max_depth:" << act.max_depth_sum - act.src_side_max_depth << std::endl;
        }
#endif //#if DEPTH_KILL_ENABLE == 1
        else
        {
            Assert(0, "invalid value returned from BiBFSBaseline::run:" + std::to_string(result));
        }
    }

    return result;
}

long long bfs(long long stepNum, Graph &g, int vid, std::map<int, VisitedNode *> &v2visited)
{
    int v_ptr = vid;
    VisitedNode *firstVisited = new VisitedNode(v_ptr, 0);
    std::vector<VisitedNode *> currentLevelVisited;
    currentLevelVisited.push_back(firstVisited); //当前层将要访问的顶点
    v2visited[v_ptr] = firstVisited;
    long long step;
    for (step = 1; currentLevelVisited.empty() == 0 && step <= stepNum; step++)
    {
#if TIME_KILL_ENABLE == 1
        if (is_time_out)
        {
            return -1;
        }
#endif //#if TIME_KILL_ENABLE == 1
        std::vector<VisitedNode *> nextLevelVisited;
        for (size_t i = 0; i < currentLevelVisited.size(); i++)
        {
            VisitedNode *curVisited = currentLevelVisited[i];

            std::vector<int> eList = g.get_neighbors(curVisited->v); // neighbors

            for (size_t j = 0; j < eList.size(); j++)
            {
                int nextV = eList[j];
                VisitedNode *nextVisited = v2visited[nextV];
                //未曾访问过
                if (nextVisited == NULL)
                {
                    nextVisited = new VisitedNode(nextV, step);
                    if (step < stepNum)
                    {
                        nextLevelVisited.push_back(nextVisited);
                        //将作为下一轮循环的currentLevelVisited，每个都会取出来进行扩展处理
                        //扩展处理：对于其所有入（出）邻居，给邻居addPreviousVisited，并且把还没有扩展过的顶点入队之后来做扩展
                        //对每个顶点做了一次扩展处理之后不用再做一次，因为某特定顶点的入（出）邻居不会变，首次扩展处理后已经给其所有入（出）邻居addPreviousVisited并把所有未做过扩展处理的顶点入队了
                    }
                    // step == stepNum时是到一定长度路径的端点了，端点不用做扩展处理
                    v2visited[nextV] = nextVisited;
                }
                nextVisited->addPreviousVisited(curVisited);
            }
        }
        if (step < stepNum)
        {
            currentLevelVisited = nextLevelVisited;
        }
    }
    // currentLevelVisited.empty() || step > stepNum
    return step;
}
