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

/*g: general graph*/
int solve_2VDPP(MemGraph &g, VID_TYPE s, VID_TYPE t);

// g is 2connected
// return 2VDPP has solution or not, if yes return solution in path1 and path2
// return -1(TIME_EXCEED_RESULT) if time exceed
// path1 and path2 are passed as empty
int solve_on_2connected(MemGraph &g, VID_TYPE s, VID_TYPE t, std::back_insert_iterator<std::vector<VID_TYPE>> path1_back_it, std::back_insert_iterator<std::vector<VID_TYPE>> path2_back_it);

// do bctree decomposing from s, stop when finishing block(s)
// return whether block(s)==block(t), and if so, write block(s) to g and return new2old mapping
// return -1(TIME_EXCEED_RESULT) if time exceed
int st_biconnected_component(MemGraph &g, VID_TYPE &s, VID_TYPE &t);

/*return:
    1:solved
    0:no solution
    -1(TIME_EXCEED_RESULT): time limit exceed*/
enum Remove2VCutSel
{
    REMOVE_S, // return path s->t
    REMOVE_T  // return path t->s
};
int remove_2vCut_containing_s(Remove2VCutSel sel, MemGraph &g, VID_TYPE s, VID_TYPE t, std::back_insert_iterator<std::vector<VID_TYPE>> path1_back_it, std::back_insert_iterator<std::vector<VID_TYPE>> path2_back_it);

struct bctreeNode
{
    int parent;         // parent idx(parent comp number too) -1:no parent
    VID_TYPE cut_point; // curr->parent, cutpoint(vid) on this edge
    bctreeNode() : parent(-1), cut_point(-1) {}
    bctreeNode(int cutv) : parent(-1), cut_point(cutv) {}
    bctreeNode(int p, int cutv) : parent(p), cut_point(cutv) {}
};

/*build_bctree rooted at t or b(t)
return t_is_cutpoint, or -1 time exceed
return by reference(passed in as empty):
    std::vector<bctreeNode> bctree;                        // idx is comp number too, parent pointer representation
    int t_comp;                                            // if t is not cutpoint: which comp is block(t); if is cutpoint: we set -1
    std::vector<std::set<int>> comps_V;                    // comp number->comps vertex set
    std::vector<std::vector<int>> s_adj_comp;              // idx correspond to s_neighbors: which comp(s) is s_neighbors[i] in(note that if s_neighbors[i] is cutpoint, more than 1 located comp; else 1 located comp)
each bctreeNode.parent==-1 will have bctreeNode.cut_point==t
(eg,if t is not cut point: will still have bctreeNode(block(t)).cut_point==t)
*/
int build_bctree(Graph &g, VID_TYPE t, const std::vector<VID_TYPE> s_neighbors, std::vector<bctreeNode> &bctree, int &t_comp, std::vector<std::unordered_set<VID_TYPE>> &comps_V, std::vector<std::vector<int>> &s_adj_comp);
#if DEBUG_LEVEL <= DEBUG
void print_bctree_vector(const std::vector<bctreeNode> &bctree);
void print_comps_V(const std::vector<std::unordered_set<VID_TYPE>> &comps_V);
void print_s_adj_comp(const std::vector<std::vector<int>> &s_adj_comp, const std::vector<VID_TYPE> &s_neighbors);
#endif //#if DEBUG_LEVEL <= DEBUG

// if t_comp>=0: t is not cutpoint, then every comp has root t_comp in bctree: return the "root" one step ahead of t_comp
// else: t is cut point, then bctree is a forest: return the truly root(in this case, -1 can be viewed as a virtual root for all trees in the forest)
int get_root_comp(int comp, const std::vector<bctreeNode> &bctree, int t_comp);
/*
return "left first" 2 s_adj_comp(whose correponding s_nbr are different) in s_adj_comp that shares no common ancestor until t_comp(with respect to bctree)
(t_comp and t_comp's ancestors could be their common ancestor; especially, when t_comp==-1, this can be view as a virtual root for bctree-forest)
s_nbrs that is not cutpoint are considered first
compu compv
u v: the correponding s_nbr idx
*/
void no_common_ancestor(int &compu, int &compv, VID_TYPE &uidx, VID_TYPE &vidx, const std::vector<std::vector<int>> &s_adj_comp, const std::vector<bctreeNode> &bctree, int t_comp);

/*
return the parent path in g from start_comp to end_cut_point in bctree
each path segment: cutpoint->cutpoint(1st seg: start_v->cutpoint), only visting the correspoding comp

note that start_v==bctreeNode(start_comp).cut_point is possible, in which case 1st seg(start_v->cutpoint) would contain only 1 vertex
*/
void get_parent_path(int start_comp, VID_TYPE start_v, VID_TYPE end_cut_point, std::back_insert_iterator<std::vector<VID_TYPE>> path_back_it, Graph &g, const std::vector<bctreeNode> &bctree, const std::vector<std::unordered_set<VID_TYPE>> &comps_V);

/*
return the parent path consisting of cut points from start_v in start_comp to root t in bctree
2nd last in cppath is:
t is cut point: (t)-root_block<-(parent cut point A of blocku)-blocku<-...
    then:A
t is not cut point: blockt<-(parent cut point A of blocku)-blocku<-...
    then:A
*/
void get_cut_point_path(int start_comp, VID_TYPE start_v, VID_TYPE t, const std::vector<bctreeNode> &bctree, std::back_insert_iterator<std::vector<VID_TYPE>> cppath_back_it);

// WARN!!未实现 未测试
/*remove all 2-vertex-cut in g
given g: 2connected, any 2-vertex-cut excludes s or t
return: 1:solved 0:no solution -1(TIME_EXCEED_RESULT): time limit exceed*/
int remove_2vCut(Graph &g, VID_TYPE s, VID_TYPE t, std::back_insert_iterator<std::vector<VID_TYPE>> path1_back_it, std::back_insert_iterator<std::vector<VID_TYPE>> path2_back_it);

#endif //_TWOCONNECTED_H