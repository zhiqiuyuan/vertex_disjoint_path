/*solve on <=2 connected, or reduce to 3 connected*/

#ifndef _TWOCONNECTED_H
#define _TWOCONNECTED_H
#include "Graph.h"
/*CONVENTION:
1.返回的路径都包含端点，由调用者进行自行处理
2.back_inserter都是对空向量进行填充，然后调用者进行拼接，拒绝传入有内容的路径向量的back_inserter，因为要考虑顺序较为麻烦
3.只有write_graph可以修改st
4.返回的路径是在 传入的g 的顶点id下的路径*/

/* 整体调用过程：不断根据情况reduce向下，直到direct solvable case，然后一路返回
（所以这个不是类似于搜索，而是根据图的情况有确定单一"求解路径"的，使用递归主要是reduce回来的时候方便构造路径）
STEP1: reduce to 3 connected
    remove 2-vertex-cut containing s
    remove 2-vertex-cut containing t*/

// WARN!!未测试
/*g: general graph*/
void solve_2VDPP(Graph &g, int s, int t);

// WARN!!未测试
// g is 2connected
// return 2VDPP has solution or not, if yes return solution in path1 and path2
// path1 and path2 are passed as empty
bool solve_on_2connected(Graph &g, int s, int t, std::back_insert_iterator<std::vector<int>> path1_back_it, std::back_insert_iterator<std::vector<int>> path2_back_it);

// do bctree decomposing from s, stop when finishing block(s)
// return whether block(s)==block(t), and if so, write block(s) to g and return new2old mapping
bool st_biconnected_component(Graph &g, int &s, int &t, std::unordered_map<int, int> &new2old);

// WARN!!未测试
/*return:
    1:solved
    0:no solution*/
enum Remove2VCutSel
{
    REMOVE_S,
    REMOVE_T
};
int remove_2vCut_containing_s(Remove2VCutSel sel, Graph &g, int s, int t, std::back_insert_iterator<std::vector<int>> path1_back_it, std::back_insert_iterator<std::vector<int>> path2_back_it);

struct bctreeNode
{
    int parent;    // parent idx(parent comp number too)
    int cut_point; // curr->parent, cutpoint(vid) on this edge
    bctreeNode() : parent(-1), cut_point(-1) {}
    bctreeNode(int cutv) : parent(-1), cut_point(cutv) {}
};

/*build_bctree rooted at t or b(t)
return t_is_cutpoint
return by reference(passed in as empty):
    std::vector<bctreeNode> bctree;                        // idx is comp number too, parent pointer representation
    int t_comp;                                            // if t is not cutpoint: which comp is block(t); if is cutpoint: we set -1
    std::vector<std::set<int>> comps_V;                    // comp number->comps vertex set
    std::vector<std::vector<int>> s_adj_comp;              // idx correspond to s_neighbors: which comp(s) is s_neighbors[i] in(note that if s_neighbors[i] is cutpoint, more than 1 located comp; else 1 located comp)
each bctreeNode.parent==-1 will have bctreeNode.cut_point==t
(eg,if t is not cut point: will still have bctreeNode(block(t)).cut_point==t)
*/
bool build_bctree(Graph &g, int t, const std::vector<int> s_neighbors, std::vector<bctreeNode> &bctree, int &t_comp, std::vector<std::set<int>> &comps_V, std::vector<std::vector<int>> &s_adj_comp);
#if DEBUG_LEVEL <= DEBUG
void print_bctree_vector(const std::vector<bctreeNode> &bctree);
void print_comps_V(const std::vector<std::set<int>> &comps_V);
void print_s_adj_comp(const std::vector<std::vector<int>> &s_adj_comp, const std::vector<int> &s_neighbors);
#endif //#if DEBUG_LEVEL <= DEBUG

// WARN!!未实现 未测试
/*
return "left first" 2 s_adj_comp(whose correponding s_nbr are different) in s_adj_comp that shares no common ancestor(with respect to bctree)(except t)
s_nbrs that is not cutpoint are considered first
compu compv
u v: the correponding s_nbr
*/
void no_common_ancestor(int &compu, int &compv, int &u, int &v, const std::vector<std::vector<int>> &s_adj_comp, const std::vector<bctreeNode> &bctree);

// WARN!!未实现 未测试
/*
return the parent path in g from start_comp to end_cut_point in bctree
each path segment: cutpoint->cutpoint(1st seg: start_v->cutpoint), only visting the correspoding comp

note that start_v==bctreeNode(start_comp).cut_point is possible, in which case 1st seg(start_v->cutpoint) would contain only 1 vertex
*/
void get_parent_path(int start_comp, int start_v, int end_cut_point, std::back_insert_iterator<std::vector<int>> path_back_it, const Graph &g, const std::vector<bctreeNode> &bctree, const std::vector<std::set<int>> &comps_V);

// WARN!!未实现 未测试
/*
return the parent path consisting of cut points from start_v in start_comp to root t in bctree
*/
void get_cut_point_path(int start_comp, int start_v, int t, const std::vector<bctreeNode> &bctree, std::back_insert_iterator<std::vector<int>> cppath_back_it);

// WARN!!未实现 未测试
/*remove all 2-vertex-cut in g
given g: 2connected, any 2-vertex-cut excludes s or t*/
int remove_2vCut(Graph &g, int s, int t, std::back_insert_iterator<std::vector<int>> path1_back_it, std::back_insert_iterator<std::vector<int>> path2_back_it);

#endif //_TWOCONNECTED_H