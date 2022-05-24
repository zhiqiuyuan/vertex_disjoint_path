#include "tools.h"
#include "DataSetGraph.h"
#include "TwoConnected.h"

/* 命令行参数
 * -g graph_file
 * -p vertex_pair_file
 * -o output_dir
 * [-c pairs_count](default: total vertex_pair_file) //st_pair_set.size()
 * [-r round, repeat_times](default: 1) //(for time measuring) for a fixed st_pair set, repeat how many times
 */
void printUsage()
{
    print_with_colorln(YELLOW, "usage: ./<bin_name> -g graph_file -p vertex_pair_file -o output_dir [-c pairs_count] [-r repeat_times]");
    print_with_colorln(YELLOW, "	pairs_count: default to total vertex_pair_file");
    print_with_colorln(YELLOW, "	repeat_times: default to 1");
}

#if TIME_KILL_ENABLE == 1
std::chrono::_V2::system_clock::time_point ctrl_start;
#endif //#if TIME_KILL_ENABLE == 1

int main(int argc, char **argv)
{
    TVEGraph g, g_copy;
    // EGraph g, g_copy;

    std::string gfname, vfname, outfname;
    if (argc < 5)
    {
        printUsage();
        return -1;
    }
    std::map<std::string, std::string> cmd;
    for (int i = 1; i < argc; i += 2)
    {
        cmd[std::string(argv[i])] = std::string(argv[i + 1]);
    }
    gfname = cmd["-g"];
    vfname = cmd["-p"];
    outfname = cmd["-o"];
    if (outfname.empty() == 0 && outfname[outfname.size() - 1] != '/')
    {
        outfname += '/';
    }

    int pairs_cnt = -1;
    if (cmd.count("-c"))
    {
        pairs_cnt = std::stoi(cmd["-c"]);
        if (pairs_cnt <= 0)
        {
            pairs_cnt = -1;
        }
    }

    int repeat_times = 1;
    if (cmd.count("-r"))
    {
        repeat_times = std::stoi(cmd["-r"]);
        if (repeat_times <= 0)
        {
            repeat_times = 1;
        }
    }

    print_with_colorln(GREEN, "loading graph...");
    if (g.buildGraph(gfname) == 0)
    {
        printErrorWithLocation("buildGraph failed!", __FILE__, __LINE__);
        return -1;
    }
    g_copy = g;

    print_with_colorln(GREEN, "loading vertex pairs...");
    std::vector<std::pair<int, int>> st_pairs;
    std::ifstream fin(vfname);
    int s, t;
    for (int i = 0; ((i < pairs_cnt) || (pairs_cnt == -1)) && fin >> s >> t; ++i)
    {
        st_pairs.emplace_back(s, t);
    }
    int cnt = 0, scnt = 0, tcnt = 0; // for average computing
    double average_d = 0;            // unit: second
    double average_success_cnt = 0, average_time_exceeding_cnt = 0;
    print_with_colorln(GREEN, "searching for circle for each pair...");

    size_t idx0 = gfname.find_last_of('/') + 1, idx1 = gfname.find_last_of('.');
    std::string dgname = gfname.substr(idx0, idx1 - idx0);
#ifdef ENABLE_PROCESS_RECORD
    std::ofstream pair_record_fout("pair_record/" + dgname + "pair_record");
#endif // ENABLE_PROCESS_RECORD
    for (int i = 1; i <= repeat_times; ++i)
    {
        print_with_colorln(YELLOW, "round " + std::to_string(i) + ":");
        auto start = std::chrono::system_clock::now();
        int s, t; // start node id and target node id;
        int total_cnt = 0;
        int success_cnt = 0, time_exceeding_cnt = 0;
        int result;
        double d = 0, dd;
        for (auto pair : st_pairs)
        {
            s = pair.first;
            t = pair.second;
            print_with_colorln(GREEN, "\t" + std::to_string(++total_cnt) + ":" + std::to_string(s) + "->" + std::to_string(t));
#ifdef ENABLE_PROCESS_RECORD
            pair_record_fout << s << " " << t << std::endl;
#endif // ENABLE_PROCESS_RECORD
            result = solve_2VDPP(g, s, t);
            success_cnt += (result == 1);
            time_exceeding_cnt += (result == TIME_EXCEED_RESULT);

            // exclude g recover(through copy) time
            auto end = std::chrono::system_clock::now();
            std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            dd = double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
            d += dd;
            g = g_copy;
            start = std::chrono::system_clock::now();
        }
        moving_average(average_d, d, cnt);
        std::cout << "\ttime:" << d / (st_pairs.size()) << std::endl;
        moving_average(average_success_cnt, success_cnt, scnt);
        moving_average(average_time_exceeding_cnt, time_exceeding_cnt, tcnt);
        std::cout << "\tsuccess_cnt:" << average_success_cnt << std::endl;
        std::cout << "\ttime_exceeding_cnt:" << average_time_exceeding_cnt << std::endl;
    }
#ifdef ENABLE_PROCESS_RECORD
    pair_record_fout.close();
#endif // ENABLE_PROCESS_RECORD
    average_d = average_d / (st_pairs.size());
    std::cout << average_d << std::endl;

    outfname += dgname + ".txt";
    std::ofstream fout(outfname);
    if (fout.is_open() == 0)
    {
        print_with_colorln(RED, "file open failed:" + outfname + "\nopen this instead: " + dgname + ".txt");
        fout.open(dgname + ".txt");
        if (fout.is_open() == 0)
        {
            print_with_colorln(RED, "file open failed:" + dgname + ".txt");
            return 0;
        }
    }
    fout << average_d << std::endl;
    fout << average_success_cnt << std::endl;
    fout << average_time_exceeding_cnt << std::endl;
    fout << st_pairs.size() << std::endl;
    fout.close();
    return 0;
}