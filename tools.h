#ifndef _TOOLS_H
#define _TOOLS_H

#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <set>
#include <vector>
#include <deque>
#include <queue>
#include <string>
#include <algorithm>
#include <climits>
#include <chrono>
#include <ctime>
#include <assert.h>

#include "config.h"

void Assert(bool exp, std::string s);
void printErrorWithLocation(std::string s, const char *FILE__, int LINE__);

void print_with_color(int color_code, std::string s);
void print_with_colorln(int color_code, std::string s);
//template <typename T>
void print_vectorln(std::vector<int> &v);
//template <typename T>
void print_setln(std::set<int> &v);

//初始：cnt设置为0，new_val传第一个元素，old_ave 随意给（比如给0）
void moving_average(double &old_ave, double new_val, int &cnt);
// return [0,possible_max)
long long Rand(long long possible_max);

#endif //_TOOLS_H
