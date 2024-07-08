#pragma once
#define TERM_H

#include "orbit.h"

// enum ANSI_STYLE {
//     STYLE_Reset       = 0,

//     STYLE_Bold        = 1,
//     STYLE_Dim         = 2,
//     STYLE_Italic      = 3,

//     STYLE_FG_Black    = 30,
//     STYLE_FG_Red      = 31,
//     STYLE_FG_Green    = 32,
//     STYLE_FG_Yellow   = 33,
//     STYLE_FG_Blue     = 34,
//     STYLE_FG_Magenta  = 35,
//     STYLE_FG_Cyan     = 36,
//     STYLE_FG_White    = 37,
//     STYLE_FG_Default  = 39,

//     STYLE_BG_Black    = 40,
//     STYLE_BG_Red      = 41,
//     STYLE_BG_Green    = 42,
//     STYLE_BG_Yellow   = 43,
//     STYLE_BG_Blue     = 44,
//     STYLE_BG_Magenta  = 45,
//     STYLE_BG_Cyan     = 46,
//     STYLE_BG_White    = 47,
//     STYLE_BG_Default  = 49,
// };

// #define set_style(style) printf("\x1b[%dm", style)


// #define NO_STYLE

// use C's string literal concatenation
#ifdef NO_STYLE

#define STYLE_Reset  ""
#define STYLE_Bold   ""
#define STYLE_Dim    ""
#define STYLE_Italic ""

#define STYLE_FG_Black    ""
#define STYLE_FG_Red      ""
#define STYLE_FG_Green    ""
#define STYLE_FG_Yellow   ""
#define STYLE_FG_Blue     ""
#define STYLE_FG_Magenta  ""
#define STYLE_FG_Cyan     ""
#define STYLE_FG_White    ""
#define STYLE_FG_Default  ""

#define STYLE_BG_Black    ""
#define STYLE_BG_Red      ""
#define STYLE_BG_Green    ""
#define STYLE_BG_Yellow   ""
#define STYLE_BG_Blue     ""
#define STYLE_BG_Magenta  ""
#define STYLE_BG_Cyan     ""
#define STYLE_BG_White    ""
#define STYLE_BG_Default  ""

#else

#define STYLE_Reset  "\x1b[0m"
#define STYLE_Bold   "\x1b[1m"
#define STYLE_Dim    "\x1b[2m"
#define STYLE_Italic "\x1b[3m"

#define STYLE_FG_Black    "\x1b[30m"
#define STYLE_FG_Red      "\x1b[31m"
#define STYLE_FG_Green    "\x1b[32m"
#define STYLE_FG_Yellow   "\x1b[33m"
#define STYLE_FG_Blue     "\x1b[34m"
#define STYLE_FG_Magenta  "\x1b[35m"
#define STYLE_FG_Cyan     "\x1b[36m"
#define STYLE_FG_White    "\x1b[37m"
#define STYLE_FG_Default  "\x1b[39m"

#define STYLE_BG_Black    "\x1b[40m"
#define STYLE_BG_Red      "\x1b[41m"
#define STYLE_BG_Green    "\x1b[42m"
#define STYLE_BG_Yellow   "\x1b[43m"
#define STYLE_BG_Blue     "\x1b[44m"
#define STYLE_BG_Magenta  "\x1b[45m"
#define STYLE_BG_Cyan     "\x1b[46m"
#define STYLE_BG_White    "\x1b[47m"
#define STYLE_BG_Default  "\x1b[49m"

#endif