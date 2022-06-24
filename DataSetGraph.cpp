#include "DataSetGraph.h"

bool TVEGraph::buildGraph(std::string fname)
{
    std::ifstream fin(fname);
    std::cout << fname << std::endl;
    if (fin.is_open() == 0)
    {
        ("file open failed:" + fname);
        return 0;
    }

    char ty;
    VID_TYPE vertexNum;
    EID_TYPE edgeNum;
    fin >> ty >> vertexNum >> edgeNum; // t vnum enum
    vertex_num = vertexNum;
    neighbors.resize(vertex_num);

    std::string no_use;
    std::getline(fin, no_use); // t vnum enum line
    for (VID_TYPE i = 0; i < vertexNum; ++i)
    {
        std::getline(fin, no_use); // absorb line
        /*
        if (no_use.empty() || no_use.substr(0, 2) != "v ")
        {
            printErrorWithLocation("read graph problem: real vertex data less then specified at the front of file!", __FILE__, __LINE__);
            fin.close();
            return 0;
        }
        */
    }
    VID_TYPE s, t;
    for (EID_TYPE i = 0; i < edgeNum; ++i)
    {
        fin >> ty >> s >> t;
        // std::cout << ty << " " << s << " " << t << std::endl;
        std::getline(fin, no_use); // absorb remaining line
        if (ty != 'e')
        {
            printErrorWithLocation("read graph problem: not fitting 'e sid tid' format!", __FILE__, __LINE__);
            fin.close();
            return 0;
        }
        // s--t
        // 避免重边
        std::vector<VID_TYPE> &snb = neighbors[s];
        std::vector<VID_TYPE> &tnb = neighbors[t];
        if (std::find(snb.begin(), snb.end(), t) == snb.end())
        {
            snb.push_back(t);
        }
        if (std::find(tnb.begin(), tnb.end(), s) == tnb.end())
        {
            tnb.push_back(s);
        }
    }
    fin.close();
    return 1;
}

bool EGraph::buildGraph(std::string fname)
{
    std::ifstream fin(fname);
    if (fin.is_open() == 0)
    {
        ("file open failed:" + fname);
        return 0;
    }
    // s->t
    unsigned s, t;
    // std::string no_use;
    // std::map<VID_TYPE, std::vector<VID_TYPE>> map_neighbors;
    // EID_TYPE i = 0; // edge record count
    while (fin >> s >> t)
    {
        // std::getline(fin, no_use); // absorb the rest line
        add_edge(s, t);

        /*// 避免重边
        std::vector<VID_TYPE> &snb = map_neighbors[s];
        std::vector<VID_TYPE> &tnb = map_neighbors[t];
        if (std::find(snb.begin(), snb.end(), t) == snb.end())
        {
            snb.push_back(t);
        }
        if (std::find(tnb.begin(), tnb.end(), s) == tnb.end())
        {
            tnb.push_back(s);
        }
        */

        /*++i;
        if (i % 100000 == 0)
        {
            std::cout << "read " << i << " edges done." << std::endl;
        }
        */
    }
    /*// copy to neighbors
    vertex_num = map_neighbors.size();
    neighbors.resize(vertex_num);
    for (auto &it : map_neighbors)
    {
        neighbors[it.first] = it.second;
    }
    */

    fin.close();
    return 1;
}