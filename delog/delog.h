#ifndef _WMJ_DELOG_H_
#define _WMJ_DELOG_H_

/**
 * DEBUG开关：
 * |-------------|---------------------------------------------|
 * |DEBUG|RELEASE| 结果                                         |
 * |-----|-------|---------------------------------------------|
 * |  0  |   x   | 仅将日志输出到文件中                           |
 * |  1  |   0   | 存在检测和日志文件，并将检测数据和日志都输出到屏幕上 |
 * |  1  |   1   | 存在检测和日志文件，无屏幕输出                   |
 * |-----|-------|---------------------------------------------|
 **/
#ifdef DEBUG
    #ifndef RELEASE
        #define DELOG_MODE 0
    #else 
        #define DELOG_MODE 1
    #endif
#else
    #define DELOG_MODE 2
#endif

// #include <stdarg.h>
#include "types.h"
#include <stdio.h>
#include <stdarg.h>

#define __info(fmt, ...) do {\
    printf("[%s]<%s>(%d):", __FILE__, __func__, __LINE__);\
    printf(fmt, __VA_ARGS__);\
    putchar('\n');\
} while(0)

#define mkstr(symbol) #symbol
#define _POS_INFOBITS (_I)1
#define _NPOS_INFOBITS (~_POS_INFOBITS)

enum {
    _INIT_INFO_DELOG = 0,
    _LOG_INFO_DELOG = 2,
    _DBG_INFO_DELOG = 4,
    _MAX_INFO_DELOG,
};
#define _pos_delog(type, ...) _wmj_delog((type)|_POS_INFOBITS, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define _Npos_delog(type, ...) _wmj_delog((type)&_NPOS_INFOBITS, __VA_ARGS__)
// #define init_delog() do{_Npos_delog(_INIT_INFO_DELOG, 0);} while(0)
#define init_delog(dbgname, logname) do{_Npos_delog(_INIT_INFO_DELOG, dbgname, logname);}while(0)
#if DELOG_MODE < 2
#define _debug_info(expstr, m, ...) _pos_delog(_DBG_INFO_DELOG, m, expstr, __VA_ARGS__)
#define _dbg_log(...) _pos_delog(_DBG_INFO_DELOG, "t", "log", __VA_ARGS__)
#define _d_end() _dbg_log("finished.")
#else
#define _debug_info(expstr, m, ...) do{}while(0)
#define _dbg_log(...) do{}while(0)
#define _d_end() do{}while(0)
#endif
#define _log_info(...) _Npos_delog(_LOG_INFO_DELOG, __VA_ARGS__)
#define _reg() _log_info("[%s] run!", __func__)


// 打印一段地址空间
#define _sa(exp, len) _debug_info(mkstr(exp), "a", exp, len)
// 以十进制打印无符号一个整数
#define _sI(exp) _debug_info(mkstr(exp), "I", exp)
// 以十六进制打印一个整数
#define _sx(exp) _debug_info(mkstr(exp), "x", (_u32)exp)
// 打印一个浮点数
#define _sf(exp) _debug_info(mkstr(exp), "f", (double)exp)
// 打印一个字符串
#define _ss(exp) _debug_info(mkstr(exp), "s", (_s)exp)
// 打印一个变量的地址
#define _sp(exp) _debug_info(mkstr(exp), "p", &(exp))
// 打印一个字符
#define _sc(exp) _debug_info(mkstr(exp), "c", (_i32)(exp))
// 以十进制打印一个带符号整数
#define _si(exp) _debug_info(mkstr(exp), "d", (_i32)exp)
// 打印当前位置
#define _pos() _debug_info("", "n", "")

#include <stdlib.h>
#define _error_gs(exp, escape_label, ...) do {\
    if (exp) {\
        _debug_info(mkstr(exp), "e", __VA_ARGS__);\
        goto escape_label;\
    }\
} while(0)

#define _error_g(exp, escape_label) do {\
    if (exp) {\
        _debug_info(mkstr(exp), "e", "An error occurs!");\
        goto escape_label;\
    }\
} while(0)

#define _error_s(exp, ...) do {\
    if (exp) {\
        _debug_info(mkstr(exp), "e", __VA_ARGS__);\
        exit(1);\
    }\
} while(0)

#define _error(exp) do {\
    if (exp) {\
        _debug_info(mkstr(exp), "e", "An error occurs!");\
        exit(1);\
    }\
} while(0)

#define _assert(exp) do {\
    if (!(exp)) {\
        _debug_info(mkstr(exp), "e", "Assert fail!");\
        exit(1);\
    }\
} while(0)


#define _bug(exp) do {\
    if (exp) {\
        _debug_info(mkstr(exp), "e", "found bug!");\
        exit(2);\
    }\
} while(0);


void _wmj_delog(_I type, ...);

/***
 * 将p所指位置连续len个字节的单元的值取出，以16进制放在str中，size为str的最大长度
 */
_I print_bin(_s str, _I size, _u8 *p, _I len);

#endif