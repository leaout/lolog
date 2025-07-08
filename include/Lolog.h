/**
* Created by chenly on 1/2/24.
* Description
**/


#ifndef LOLOG_LOLOG_H
#define LOLOG_LOLOG_H

#include <string>
#include <sstream>
// fmt
#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <fmt/xchar.h>

#ifdef __linux
#define __LOFILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#else
#define __LOFILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#endif

namespace lolog {
    void init_logging(const char *log_path_name, int log_remain_counts, int log_file_size = 200 << 20);
    void set_color(bool);
    void set_std_out(bool);
    void set_print_file_line(bool);
    //Fatal = 0, Error = 1, Warn = 2, Info = 3, Debug = 4
    void set_log_level(int);
    void set_log_level(std::string);
    //LogTypeDateTime = 0, LogTypeDate = 1, LogTypeAutoIncSn = 2
    void set_log_formate(int);

    template <typename... Args>
    void debug(const char *fmt, Args... args);
    template <typename... Args>
    void info(const char *fmt, Args... args);
    template <typename... Args>
    void warn(const char *fmt, Args... args);
    template <typename... Args>
    void error(const char *fmt, Args... args);
    template <typename... Args>
    void fatal(const char *fmt, Args... args);

    template <typename... Args>
    void debugex(const char *file_name, int line, const char *fmt, Args... args);
    template <typename... Args>
    void infoex(const char *file_name, int line, const char *fmt, Args... args);
    template <typename... Args>
    void warnex(const char *file_name, int line, const char *fmt, Args... args);
    template <typename... Args>
    void errorex(const char *file_name, int line, const char *fmt, Args... args);
    template <typename... Args>
    void fatalex(const char *file_name, int line, const char *fmt, Args... args);
}
#define LODEBUG(param, args...) lolog::debugex(__LOFILENAME__, __LINE__, param, ##args)
#define LOINFO(param, args...) lolog::infoex(__LOFILENAME__, __LINE__, param, ##args)
#define LOWARN(param, args...) lolog::warnex(__LOFILENAME__, __LINE__, param, ##args)
#define LOERROR(param, args...) lolog::errorex(__LOFILENAME__, __LINE__, param, ##args)
#define LOFATAL(param, args...) lolog::fatalex(__LOFILENAME__, __LINE__, param, ##args)

#endif //LOLOG_LOLOG_H
