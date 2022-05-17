#include "tools.h"

void print_with_color(int color_code, std::string s)
{
    std::cout << ADD_COLOR(color_code, s);
}
void print_with_colorln(int color_code, std::string s)
{
    std::cout << ADD_COLOR(color_code, s) << std::endl;
}
// template <typename T>
void print_vectorln(std::vector<int> &v)
{
    for (int i : v)
    {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}
// template <typename T>
void print_setln(std::set<int> &v)
{
    for (int i : v)
    {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}

void Assert(bool exp, std::string s)
{
    print_with_colorln(RED, s);
    assert(exp);
}

void printErrorWithLocation(std::string s, const char *FILE__, int LINE__)
{
    print_with_colorln(RED, std::string(FILE__) + " " + std::to_string(LINE__) + "\n" + s);
}

void moving_average(double &old_ave, double new_val, int &cnt)
{
    ++cnt;
    double tmp1 = old_ave * ((double(cnt - 1)) / cnt);
    double tmp2 = new_val / cnt;
    old_ave = tmp1 + tmp2;
}

long long Rand(long long possible_max)
{
    // segment division
    double min_interval_width = possible_max / (double)RAND_MAX;
    long long re = rand();
    while (min_interval_width >= RAND_MAX)
    {
        re = re * RAND_MAX + rand();
        min_interval_width /= RAND_MAX;
    }
    return re % possible_max;
}