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
#include <functional>
#include <future>
#include <thread>
#include <assert.h>
#include <pthread.h>

#include "config.h"
//#include "dbengine/rocksdb.h"

void Assert(bool exp, std::string s);
void printErrorWithLocation(std::string s, const char *FILE__, int LINE__);

void print_with_color(int color_code, std::string s);
void print_with_colorln(int color_code, std::string s);
// template <typename T>
void print_vectorln(std::vector<VID_TYPE> &v);
// template <typename T>
void print_setln(std::set<VID_TYPE> &v);
void print_queueln(std::queue<VID_TYPE> q);
void print_dequeln(std::deque<VID_TYPE> q);
void print_umapln(const std::unordered_map<VID_TYPE, VID_TYPE> &mp);

//初始：cnt设置为0，new_val传第一个元素，old_ave 随意给（比如给0）
void moving_average(double &old_ave, double new_val, int &cnt);
// return [0,possible_max)
long long Rand(long long possible_max);

void map_new2old(int start_idx, const std::vector<VID_TYPE> &newv, std::back_insert_iterator<std::vector<VID_TYPE>> old_back_it, std::unordered_map<VID_TYPE, VID_TYPE> &new2old);

#endif //_TOOLS_H
