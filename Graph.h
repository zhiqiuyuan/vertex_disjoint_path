#ifndef _GRAPH_H
#define _GRAPH_H

#include "tools.h"
// v in file are vid
// no parallel edge
class Graph
{
protected:
    VID_TYPE vertex_num;

public:
    Graph() : vertex_num(0) {}
    VID_TYPE vertexnum() { return vertex_num; }
    virtual void add_edge(VID_TYPE s, VID_TYPE t) = 0;
    virtual VID_TYPE get_degree(VID_TYPE vid) = 0;
    virtual std::vector<VID_TYPE> get_neighbors(VID_TYPE vid) = 0;

    // return build succeed or not
    virtual bool buildGraph(std::string fname) = 0;

    std::vector<VID_TYPE> intersect_neighbors(VID_TYPE s, VID_TYPE t)
    {
        assert(s >= 0 && t >= 0);
        std::vector<VID_TYPE> nbrs1 = get_neighbors(s), nbrs2 = get_neighbors(t);
        std::sort(nbrs1.begin(), nbrs1.end());
        std::sort(nbrs2.begin(), nbrs2.end());
        std::vector<VID_TYPE> re;
        int i1 = 0, i2 = 0;
        int e1 = nbrs1.size(), e2 = nbrs2.size();
        while (i1 < e1 && i2 < e2)
        {
            if (nbrs1[i1] == nbrs2[i2])
            {
                re.push_back(nbrs1[i1]);
                i1++;
                i2++;
            }
            else if (nbrs1[i1] < nbrs2[i2])
            {
                i1++;
            }
            else
            {
                i2++;
            }
        }
        return re;
    }
    void print_graph();
    void generate_rand_vpairs(int pairs_cnt, std::set<std::pair<VID_TYPE, VID_TYPE>> &st_pairs);
};

//  degree==0 then this vertex is not included in vertex_num
class MemGraph : public Graph
{
protected:
    std::vector<std::vector<VID_TYPE>> neighbors;

public:
    MemGraph() { vertex_num = 0; }
    void add_vertex(VID_TYPE s)
    {
        if (s >= neighbors.size())
        {
            ++vertex_num;
            neighbors.resize(s + 1, {});
        }
        else if (neighbors[s].empty())
        {
            ++vertex_num;
        }
    }
    void add_edge(VID_TYPE s, VID_TYPE t)
    {
        assert(s >= 0 && t >= 0);
        add_vertex(s);
        add_vertex(t);
        neighbors[s].push_back(t);
        neighbors[t].push_back(s);
    }
    VID_TYPE get_degree(VID_TYPE vid)
    {
        assert(vid >= 0 && vid < neighbors.size());
        return neighbors[vid].size();
    }
    std::vector<VID_TYPE> get_neighbors(VID_TYPE vid)
    {
        assert(vid >= 0 && vid < neighbors.size());
        return neighbors[vid];
    }
    std::vector<VID_TYPE> delete_vertex(VID_TYPE vid)
    {
        assert(vid >= 0 && vid < neighbors.size() && neighbors[vid].size());
        for (auto iter = neighbors.begin(); iter != neighbors.end(); ++iter)
        {
            std::vector<VID_TYPE> &nbrs = *iter;
            std::vector<VID_TYPE>::iterator it;
            if ((it = std::find(nbrs.begin(), nbrs.end(), vid)) != nbrs.end())
            {
                nbrs.erase(it); //不会释放内存
            }
        }
        std::vector<VID_TYPE> tmp = neighbors[vid];
        neighbors[vid].clear(); //不会释放内存
        --vertex_num;
        return tmp;
    }
    void recover_vertex(VID_TYPE vid, const std::vector<VID_TYPE> &nbrs)
    {
        assert(vid >= 0 && vid < neighbors.size());
        neighbors[vid] = nbrs;
        for (VID_TYPE v : nbrs)
        {
            neighbors[v].push_back(vid);
        }
        ++vertex_num;
    }

    // create new adjancent list representation in new_neighbors then copy
    // new g: vertex in V, all edges in g between V
    // return new g
    void write_graph(const std::vector<VID_TYPE> &V);
    void write_graph(const std::unordered_set<VID_TYPE> &V);

    // load total graph into std::vector<std::vector<VID_TYPE>> neighbors
    virtual bool buildGraph(std::string fname) = 0;
};

#define END_OF_VERTEX (VID_TYPE)(-1)
// if next_vid==END_OF_VERTEX: there's no more vertex
// a record (key:END_OF_VERTEX, value:begin_vid) is stored in db
#define HEADER_LEN 3 // header len in each value: next vertex; degree; first neighbor's index in neighbors

/*
class DBGraph : public Graph
{
protected:
    rocksdb::DB *db;
    std::string db_path;

public:
    DBGraph(std::string db_path0) : db_path(db_path0)
    {
        rocksdb::Options options;
        options.create_if_missing = false;
        rocksdb::Status status = rocksdb::DB::Open(options, db_path0, &db);

        // missing: create db
        if (status.ok() == 0)
        {
            std::cout << status.ToString() << std::endl;

            // free
            delete db;

            options.create_if_missing = true;
            status = rocksdb::DB::Open(options, db_path0, &db);
            assert(status.ok());
        }
        vertex_num = 0;
    }
    rocksdb::Slice MAKE_KEY_SLICE(VID_TYPE s)
    {
        return rocksdb::Slice((const char *)&s, sizeof(VID_TYPE));
    }
    void str2nbrs(std::string &slice, std::vector<VID_TYPE> &data)
    {
        VID_TYPE *d = (VID_TYPE *)slice.c_str();
        size_t pos = d[2];
        size_t deg = d[1];
        d += HEADER_LEN; // now d starts at nbr
        for (size_t i = 0; i < deg; ++i)
        {
            // std::cout << "\t" << d[2*pos] << std::endl;
            data.push_back(d[2 * pos]);
            pos = d[2 * pos + 1];
        }
    }
    void print_value(const char *str, size_t bytes_len)
    {
        const VID_TYPE *d = (const VID_TYPE *)str;
        size_t len = bytes_len / sizeof(VID_TYPE);
        for (size_t i = 0; i < len; ++i)
        {
            std::cout << d[i] << " ";
        }
        std::cout << std::endl;
    }
    rocksdb::Status add_nbr(VID_TYPE s, VID_TYPE nbr)
    {
        std::string info;
        rocksdb::Slice skey = MAKE_KEY_SLICE(s);
        rocksdb::Status status = db->Get(rocksdb::ReadOptions(), skey, &info);
        if (status.ok())
        {
            // add nbr
            size_t sz = info.size();
            info.resize(sz + 2 * sizeof(VID_TYPE));

            const char *str = info.c_str();
            VID_TYPE deg = *((VID_TYPE *)(str + sizeof(VID_TYPE)));
            *((VID_TYPE *)(str + sz)) = nbr;
            *((VID_TYPE *)(str + sz + sizeof(VID_TYPE))) = deg + 1; // next nbr idx

            // update degree
            *((VID_TYPE *)(str + sizeof(VID_TYPE))) = deg + 1;

            // print_value(str, sz + 2 * sizeof(VID_TYPE));

            status = db->Put(rocksdb::WriteOptions(), skey, rocksdb::Slice(str, info.size()));
        }
        else if (status.IsNotFound())
        {
            ++vertex_num;
            // next_vid, degree, first_nbr_idx, nbr, next_nbr_idx
            VID_TYPE value[HEADER_LEN + 2] = {END_OF_VERTEX, 1, 0, nbr, 1};
            status = db->Put(rocksdb::WriteOptions(), skey, rocksdb::Slice((const char *)value, (HEADER_LEN + 2) * sizeof(VID_TYPE)));
        }
        return status;
    }

    void add_edge(VID_TYPE s, VID_TYPE t)
    {
        rocksdb::Status status = add_nbr(s, t);
        assert(status.ok());
        status = add_nbr(t, s);
        assert(status.ok());
    }
    VID_TYPE get_degree(VID_TYPE vid)
    {
        rocksdb::Slice vid_s = MAKE_KEY_SLICE(vid);
        std::string info;
        rocksdb::Status status = db->Get(rocksdb::ReadOptions(), vid_s, &info);
        assert(status.ok());
        return ((VID_TYPE *)info.c_str())[1];
    }
    std::vector<VID_TYPE> get_neighbors(VID_TYPE vid)
    {
        rocksdb::Slice vid_s = MAKE_KEY_SLICE(vid);
        std::string info;
        rocksdb::Status status = db->Get(rocksdb::ReadOptions(), vid_s, &info);
        assert(status.ok());
        std::vector<VID_TYPE> nbrs;
        str2nbrs(info, nbrs);
        return nbrs;
    }
    std::vector<VID_TYPE> delete_vertex(VID_TYPE vid)
    {
        assert(vid >= 0 && vid < vertex_num);
        for (VID_TYPE i = 0; i < vertex_num; ++i)
        {
            std::vector<VID_TYPE> &nbrs = neighbors[i];
            std::vector<VID_TYPE>::iterator it;
            if ((it = std::find(nbrs.begin(), nbrs.end(), vid)) != nbrs.end())
            {
                nbrs.erase(it);
            }
        }
        std::vector<VID_TYPE> tmp = neighbors[vid];
        neighbors[vid].clear();
        return tmp;
    }
    void recover_vertex(VID_TYPE vid, const std::vector<VID_TYPE> &nbrs)
    {
        assert(vid >= 0 && vid < vertex_num);
        neighbors[vid] = nbrs;
        for (VID_TYPE v : nbrs)
        {
            neighbors[v].push_back(vid);
        }
    }

    // create new adjancent list representation in std::vector<std::vector<VID_TYPE>> new_neighbors then copy
    // new g: vertex in V, all edges in g between V, remapping V->[0,|V|)
    // return new st and new g
    void write_graph(VID_TYPE &s, VID_TYPE &t, const std::vector<VID_TYPE> &V);
    void write_graph(VID_TYPE &s, VID_TYPE &t, const std::set<VID_TYPE> &V);

    // load total graph into std::vector<std::vector<VID_TYPE>> neighbors
    virtual bool buildGraph(std::string fname) = 0;
};
*/

#endif //_GRAPH_H