#ifndef _BIBFSBASELINE_H
#define _BIBFSBASELINE_H

#include "tools.h"
#include "Graph.h"

class VisitedNode
{
public:
    VID_TYPE v;
    int depth_bfs;
    int previous_visited_inBFS_sz;
    std::vector<VisitedNode *> previous_visited_inBFS;
    VisitedNode(VID_TYPE _v, int _depth)
    {
        this->v = _v;
        this->depth_bfs = _depth;
    }

    void addPreviousVisited(VisitedNode *_visited)
    {
        previous_visited_inBFS.push_back(_visited);
    }

    // print 1 level
    void print_1l_pvinBFS()
    {
        for (auto vn : previous_visited_inBFS)
        {
            std::cout << vn << " ";
        }
        std::cout << std::endl;
    }
};

class BiBFSBaseline
{
    // path1:第一条独立路径
    // 目前的实现中存储的path1_vSeq、path2_vSeq不包含出发点和结束点，path1_vSet包含
    // one Vertex may corresponds to more than 1 VisitedNode, that's why choosing Vertex instead of VisitedNode
    std::set<VID_TYPE> path1_vSet;
    std::deque<VID_TYPE> path1_vSeq, path2_vSeq;
    VID_TYPE path1_joint;
    Graph *G;

    /*bibfs_based dfs枚举path: 按VisitedNode.previous_visited_inBFS dfs*/
    //用于枚举path1（run()中调用）
    //返回{0,1,TIME_EXCEED_RESULT}:从joint出发的src_side、tat_side中是否存在一条路径、该路径能够找到一条独立路径
    //如果能找到则path1_vSeq和path2_vSeq是找到的结果
    int src_side_dfs(VisitedNode *start);
    int tat_side_dfs(VisitedNode *start);

    //对于一条特定的path1找一条与其独立的路径path2，避开std::set<Vertex *> path1_vSet中的顶点
    // bibfs, on first meet，标记方式采用加入队列则标记（即遍历式的单次标记），
    // （如果在去除path1中点后仍然存在从s到t的路径，这样的标记也必然可以找到这条路径，找到一条就可以了）
    //返回path1能否找到一条独立路径path2
    //如果能找到则std::deque<Vertex *> path2_vSeq是找到的结果
    //避免path1和path2都是s-t这样的路径（如果path1是这样，在find_path2中可能找到的path2也是这样）
    int find_path2();

    // print with newline
    void print_pathln(std::deque<VID_TYPE> path_vSeq, bool print_st = 0);

public:
    std::map<VID_TYPE, VisitedNode *> src_v2visited, tat_v2visited;
    long long src_side_max_depth, max_depth_sum; // tat_side_max_depth
    // max_depth in bibfs, limit path1 search depth within them
    VID_TYPE sv, tv;

    BiBFSBaseline();
    // release src_v2visited and tat_v2visited
    ~BiBFSBaseline();
    void clear_all();
    void clear_path2();
    // fill src_v2visited and tat_v2visited
    // add s and t to path1_vSet
    int bibfs(Graph &g, long long side_depth, VID_TYPE s, VID_TYPE t);

    //对于一对顶点（已经完成Find2independentPath::bibfs）枚举path；
    // return: 1:succeed 0:failed -1(TIME_EXCEED_RESULT):exceeding time
    //能否找到找两条独立路径（fill std::deque<VID_TYPE> path1_vSeq, path2_vSeq;）
    int run();

    void print_path1path2(bool print_st = 0);

#if DEBUG_LEVEL <= DEBUG
    // for debug
    // print src_v2visited and tat_v2visited
    // 1 level previous_visited_inBFS
    void print_1l_pvinBFS();
    void print_jointln();
#endif //#if DEBUG_LEVEL <= DEBUG
};

// return: 1:succeed 0:failed -1(TIME_EXCEED_RESULT):exceeding time 3(DEPTH_LIMIT_EXCEED_RESULT):exceeding depth
// side_depth: path1单侧搜索深度 传入LLONG_MAX认为搜穿
int baseline(Graph &g, long long side_depth, VID_TYPE s, VID_TYPE t, bool print_result);

// sort joints according to src_v2visited(tat) parents total and diff num
// for print needs
struct VertexWithNum
{
    VID_TYPE v;
    int sum;
    int diff;
    VertexWithNum() : v(0), sum(0), diff(0) {}
    VertexWithNum(VID_TYPE v0, int n1, int n2) : v(v0), sum(n1 + n2)
    {
        diff = std::abs(n1 - n2);
    }
    // prior: sum max; diff max
    bool operator<(const VertexWithNum &r) const
    {
        if (sum != r.sum)
        {
            return sum > r.sum;
        }
        return diff > r.diff;
    }
};

// return max searching depth
// return -1 if timeout
long long bfs(long long stepNum, Graph &g, VID_TYPE vid, std::map<VID_TYPE, VisitedNode *> &v2visited);

#endif //_BIBFSBASELINE_H
