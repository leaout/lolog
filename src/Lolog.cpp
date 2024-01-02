/**
* Copyright (c) 2024 chenly. All rights reserved.
* Created by chenly on 1/2/24.
* Description
**/

#include <stdarg.h>

#include "../include/Lolog.h"
#include "Log.h"


namespace lolog {

    char g_log_level_def[][32] = {"[fatal] ", "[error] ", "[warn] ", "[info] ", "[debug] "};

    LogLevel g_log_level = LogLevel::Error;
    bool g_color = false;
    bool g_std_out = false;
    bool g_print_line = false;

    void init_logging(const char *log_path_name, int log_remain_counts, int log_file_size) {

        kULog.open_log_file(log_path_name);

        g_color = true;

        kULog.m_std_out = true;

        kULog.m_inc_log_size = log_file_size;

        kULog.m_log_remain_counts = log_remain_counts;

#define X(name) kLevelMap[#name] = LogLevel::name;
        LIST_OF_LOGLEVEL
#undef X

    }

    void set_color(bool set) {
        g_color = set;
    }

    void set_std_out(bool set) {
        g_std_out = set;
    }

    void set_print_file_line(bool set) {
        g_print_line = set;
    }

    void set_log_level(int set) {
        g_log_level = (LogLevel) set;
    }

    void set_log_level(std::string level) {
        auto find_ret = kLevelMap.find(level);
        if (find_ret != kLevelMap.end()) {
            g_log_level = find_ret->second;
        }
    }

    void set_log_formate(int formate) {
        kULog.set_log_file_type((LogFileType) formate);
    }

    void debugex(const char *file_name, int line, const char *fmt, ...) {
        if (g_log_level < LogLevel::Debug)
            return;

        if (g_color) {
            kULog.set_color(ULog::ColorCode::BLUE);
        }

        kULog << date_time << g_log_level_def[toUType(LogLevel::Debug)];

        kULog << "[" << file_name << ":" << line << "] ";

        va_list argptr;

        va_start(argptr, fmt);
        kULog.add_logvar_list((char *) fmt, argptr);
        va_end(argptr);

        if (g_color) {
            kULog.reset_color();
        }
        kULog << endl;
    }

    void infoex(const char *file_name, int line, const char *fmt, ...) {
        if (g_log_level < LogLevel::Info)
            return;

        if (g_color) {
            kULog.set_color(ULog::ColorCode::CYAN);
        }

        kULog << date_time << g_log_level_def[toUType(LogLevel::Info)];

        kULog << "[" << file_name << ":" << line << "] ";

        va_list argptr;

        va_start(argptr, fmt);
        kULog.add_logvar_list((char *) fmt, argptr);
        va_end(argptr);

        if (g_color) {
            kULog.reset_color();
        }
        kULog << endl;
    }

    void warnex(const char *file_name, int line, const char *fmt, ...) {
        if (g_log_level < LogLevel::Warn)
            return;

        if (g_color) {
            kULog.set_color(ULog::ColorCode::YELLOW);
        }

        kULog << date_time << g_log_level_def[toUType(LogLevel::Warn)];

        kULog << "[" << file_name << ":" << line << "] ";

        va_list argptr;

        va_start(argptr, fmt);
        kULog.add_logvar_list((char *) fmt, argptr);
        va_end(argptr);

        if (g_color) {
            kULog.reset_color();
        }
        kULog << endl;
    }

    void errorex(const char *file_name, int line, const char *fmt, ...) {
        if (g_log_level < LogLevel::Error)
            return;

        if (g_color) {
            kULog.set_color(ULog::ColorCode::RED);
        }

        kULog << date_time << g_log_level_def[toUType(LogLevel::Error)];

        kULog << "[" << file_name << ":" << line << "] ";

        va_list argptr;

        va_start(argptr, fmt);
        kULog.add_logvar_list((char *) fmt, argptr);
        va_end(argptr);

        if (g_color) {
            kULog.reset_color();
        }
        kULog << endl;
    }

    void fatalex(const char *file_name, int line, const char *fmt, ...) {
        if (g_log_level < LogLevel::Fatal)
            return;

        if (g_color) {
            kULog.set_color(ULog::ColorCode::RED);
        }

        kULog << date_time << g_log_level_def[toUType(LogLevel::Fatal)];

        kULog << "[" << file_name << ":" << line << "] ";

        va_list argptr;

        va_start(argptr, fmt);
        kULog.add_logvar_list((char *) fmt, argptr);
        va_end(argptr);

        if (g_color) {
            kULog.reset_color();
        }
        kULog << endl;
    }

    void debug(const char *fmt, ...) {
        if (g_log_level < LogLevel::Debug)
            return;

        if (g_color) {
            kULog.set_color(ULog::ColorCode::BLUE);
        }

        kULog << date_time << g_log_level_def[toUType(LogLevel::Debug)];

        va_list argptr;

        va_start(argptr, fmt);
        kULog.add_logvar_list((char *) fmt, argptr);
        va_end(argptr);

        if (g_color) {
            kULog.reset_color();
        }
        kULog << endl;
    }

    void info(const char *fmt, ...) {
        if (g_log_level < LogLevel::Info)
            return;

        if (g_color) {
            kULog.set_color(ULog::ColorCode::CYAN);
        }

        kULog << date_time << g_log_level_def[toUType(LogLevel::Info)];

        va_list argptr;

        va_start(argptr, fmt);
        kULog.add_logvar_list((char *) fmt, argptr);
        va_end(argptr);

        if (g_color) {
            kULog.reset_color();
        }
        kULog << endl;
    }

    void warn(const char *fmt, ...) {
        if (g_log_level < LogLevel::Warn)
            return;

        if (g_color) {
            kULog.set_color(ULog::ColorCode::YELLOW);
        }

        kULog << date_time << g_log_level_def[toUType(LogLevel::Warn)];

        va_list argptr;

        va_start(argptr, fmt);
        kULog.add_logvar_list((char *) fmt, argptr);
        va_end(argptr);

        if (g_color) {
            kULog.reset_color();
        }
        kULog << endl;
    }

    void error(const char *fmt, ...) {
        if (g_log_level < LogLevel::Error)
            return;

        if (g_color) {
            kULog.set_color(ULog::ColorCode::RED);
        }

        kULog << date_time << g_log_level_def[toUType(LogLevel::Error)];

        va_list argptr;

        va_start(argptr, fmt);
        kULog.add_logvar_list((char *) fmt, argptr);
        va_end(argptr);

        if (g_color) {
            kULog.reset_color();
        }
        kULog << endl;
    }

    void fatal(const char *fmt, ...) {
        if (g_log_level < LogLevel::Fatal)
            return;

        if (g_color) {
            kULog.set_color(ULog::ColorCode::RED);
        }

        kULog << date_time << g_log_level_def[toUType(LogLevel::Fatal)];

        va_list argptr;

        va_start(argptr, fmt);
        kULog.add_logvar_list((char *) fmt, argptr);
        va_end(argptr);

        if (g_color) {
            kULog.reset_color();
        }
        kULog << endl;
    }

    LogMessage::LogMessage(const char *file, int line, int log_level) {
        m_log_level = log_level;
        if ((toUType(g_log_level) >= m_log_level) && g_print_line)
            m_stream << "[" << file << ":" << line << "] ";
    }

    LogMessage::~LogMessage() {
        if (toUType(g_log_level) >= m_log_level) {
            switch ((LogLevel) m_log_level) {
                case LogLevel::Debug : {
                    debug("%s", m_stream.str().c_str());
                    break;
                }
                case LogLevel::Info : {
                    info("%s", m_stream.str().c_str());
                    break;
                }
                case LogLevel::Warn : {
                    warn("%s", m_stream.str().c_str());
                    break;
                }
                case LogLevel::Error : {
                    error("%s", m_stream.str().c_str());
                    break;
                }
                case LogLevel::Fatal : {
                    fatal("%s", m_stream.str().c_str());
                    break;
                }
            }
        }
    }
}
