#ifndef _CONFIG_H
#define _CONFIG_H

#define ENABLE_PROCESS_RECORD // enable record processed st_pairs
#define TIME_KILL_ENABLE 1
#define TIME_EXCEED_RESULT -1

// baseline: depth limit when searching path1
#define DEPTH_KILL_ENABLE 1
#define DEPTH_LIMIT_EXCEED_RESULT 3

// debug output level
#define DEBUG_LEVEL 2 // output all higher and equal
#define ERROR 4
#define WARN 3
#define INFO 2
#define DEBUG 1
#define TRACE 0

// console color
#define ADD_COLOR(color_code, s) "\u001b[" + std::to_string(color_code) + "m" + s + "\u001b[0m"
#define RED 91         // light red
#define YELLOW 93      // light yellow
#define GREEN 92       // light green
#define BLUE 94        // light blue
#define DARK_YELLOW 33 // dark yellow

#endif //_CONFIG_H