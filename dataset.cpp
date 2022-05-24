#include "tools.h"
#include "DataSetGraph.h"

// generate random st_pairs
// bin pairs_cnt gfilename pairs_dir
int main(int argc, char **argv)
{
    if (argc < 4)
    {
        std::cout << "usage: bin pairs_cnt gfilename pairs_dir" << std::endl;
        return 0;
    }
    int pairs_cnt = atoi(argv[1]);
    std::string gfname = argv[2];
    std::string outfname = argv[3];
    if (outfname.empty() == 0 && outfname[outfname.size() - 1] != '/')
    {
        outfname += '/';
    }
    // TVEGraph g;
    EGraph g;
    if (g.buildGraph(gfname) == 0)
    {
        printErrorWithLocation("buildGraph failed!", __FILE__, __LINE__);
        return -1;
    }
    std::set<std::pair<int, int>> st_pairs;
    long long n = g.vertexnum();
    long long pairs_upper = n * (n - 1) / 2;
    if (pairs_cnt > pairs_upper)
    {
        pairs_cnt = pairs_upper;
    }
    srand(time(0));
    g.generate_rand_vpairs(pairs_cnt, st_pairs);
    size_t idx = gfname.find_last_of('/');  // xxx/name.graph
    std::string s = gfname.substr(idx + 1); // name.graph
    idx = s.find_last_of('.');
    outfname += s.substr(0, idx) + ".txt";
    std::cout << "write pairs to " << outfname << "..." << std::endl;
    std::ofstream fout(outfname);
    for (auto p : st_pairs)
    {
        fout << p.first << " " << p.second << std::endl;
    }
    fout.close();
    return 0;
}
