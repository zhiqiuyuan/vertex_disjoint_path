#include "tools.h"
#include "DataSetGraph.h"
#include "TwoConnected.h"
#include "BiBFSBaseline.h"

/* 命令行参数
 * -m method (baseline, bestbound, idea)
 * -g graph_file
 * -p vertex_pair_file
 * -o output_dir
 * [-c pairs_count](default: total vertex_pair_file) //st_pair_set.size()
 * [-r round, repeat_times](default: 1) //(for time measuring) for a fixed st_pair set, repeat how many times
 * [-d side_depth](for baseline; default: LLONG_MAX)
 * [-t time_limit](default: 300)
 */
void printUsage()
{
    print_with_colorln(YELLOW, "usage: ./<bin_name> -g graph_file -p vertex_pair_file -o output_dir [-c pairs_count] [-r repeat_times]");
    print_with_colorln(YELLOW, "	pairs_count: default to total vertex_pair_file");
    print_with_colorln(YELLOW, "	repeat_times: default to 1");
}

#if TIME_KILL_ENABLE == 1
bool is_time_out;
#endif //#if TIME_KILL_ENABLE == 1

int main(int argc, char **argv)
{
    TVEGraph g, g_copy;
    // EGraph g, g_copy;

    std::string gfname, vfname, outfname, method;
    if (argc < 7)
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
    method = cmd["-m"];
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

    int time_limit_sec = 300;
    if (cmd.count("-t"))
    {
        time_limit_sec = std::stoi(cmd["-t"]);
        if (time_limit_sec <= 0)
        {
            time_limit_sec = 300;
        }
    }

    // for baseline
    long long side_depth = LLONG_MAX;
    if (cmd.count("-d"))
    {
        side_depth = std::stoi(cmd["-d"]);
        if (side_depth <= 0)
        {
            side_depth = LLONG_MAX;
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
    ///*
    for (int i = 0; ((i < pairs_cnt) || (pairs_cnt == -1)) && fin >> s >> t; ++i)
    {
        st_pairs.emplace_back(s, t);
    }
    //*/

    /*
    for (int i = 0; i < 10; ++i)
    {
        for (int j = i + 1; j < 10; ++j)
        {
            st_pairs.emplace_back(i, j);
        }
    }
    */

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
            start = std::chrono::system_clock::now();
#if TIME_KILL_ENABLE == 1
            is_time_out = 0;
            std::future<int> future;
            if (method == "bestbound")
            {
                future = std::async(std::launch::async, [&g, s, t]()
                                    { return solve_2VDPP(g, s, t); });
            }
            else if (method == "baseline")
            {
                future = std::async(std::launch::async, [&g, side_depth, s, t, i, repeat_times]()
                                    { return baseline(g, side_depth, s, t, i == repeat_times); });
            }
            else
            {
                print_with_colorln(RED, "not supported method type: " + method);
                return -1;
            }
            // Waits for the result to become available.
            // Blocks until specified timeout_duration has elapsed or the result becomes available, whichever comes first.
            // The return value identifies the state of the result.
            // WARN: the thread won't be killed after timeout!
            std::future_status status;
            do
            {
                status = future.wait_for(std::chrono::seconds(time_limit_sec));
                if (status == std::future_status::timeout)
                {
                    is_time_out = 1; // this would trigger test point return earlier
                }
                else if (status == std::future_status::deferred)
                {
                    print_with_colorln(RED, "deferred");
                    return -1;
                }
            } while (status != std::future_status::ready);
            if (is_time_out == 0)
            {
                result = future.get();
                success_cnt += (result == 1);
            }
            else
            {
                ++time_exceeding_cnt;
            }
#else  //#if TIME_KILL_ENABLE == 1
            if (method == "bestbound")
            {
                result = solve_2VDPP(g, s, t);
            }
            else if (method == "baseline")
            {
                result = baseline(g, side_depth, s, t, i == repeat_times);
            }
            else
            {
                print_with_colorln(RED, "not supported method type: " + method);
                return -1;
            }
            success_cnt += (result == 1);
#endif //#if TIME_KILL_ENABLE == 1

            // exclude g recover(through copy) time
            auto end = std::chrono::system_clock::now();
            std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            dd = double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
            d += dd;
            if (method == "bestbound")
                g = g_copy;
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