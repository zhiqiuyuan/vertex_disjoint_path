#ifndef _CONFIG_H
#define _CONFIG_H

// debug output level
#define DEBUG_LEVEL 1 // output all higher and equal
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