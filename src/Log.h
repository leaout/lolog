/**
* Copyright (c)  2024 chenly. All rights reserved.
* Created by chenly on 1/2/24.
* Description
**/

#ifndef LOLOG_LOG_H
#define LOLOG_LOG_H

#include <list>
#include <map>
#include <fstream>
#include <time.h>
#include <thread>
#include <string>
#include <sstream>
#include <unordered_map>
#include <cstring>
#include <mutex>
// fmt
#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <fmt/xchar.h>

using namespace std;

#include "CircleBuffer.h"

namespace lolog {
#define LIST_OF_LOGLEVEL \
    X(Fatal) \
    X(Error) \
    X(Warn) \
    X(Info) \
    X(Debug)

#define X(name) name,
    enum class LogLevel {
        LIST_OF_LOGLEVEL
    };
#undef X

    static std::map<std::string, enum LogLevel> kLevelMap;


    enum class LogFileType {
        DateTime = 0, Date = 1, AutoIncSn = 2
    };

    template<typename E>
    constexpr auto toUType(E enumerator) noexcept {
        return static_cast<std::underlying_type_t<E>>(enumerator);
    }

    class ULog {
    public:
        enum class ColorCode {
            BLACK = 30,
            RED = 31,
            GREEN = 32,
            YELLOW = 33,
            BLUE = 34,
            MAGENTA = 35,
            CYAN = 36,
            WHITE = 37
        };
    public:
        ULog();

        ~ULog();

        void set_log_file_type(LogFileType nLogFileType = LogFileType::DateTime);

        bool open_log_file(const char *log_path_name);

        ULog &add_log(const char *pszFmt, ...); 
        
        // fmt format
        template <typename... Args>
        ULog &add_log(const char *format, const Args &...args){
            get_log_buffer_by_id(get_current_thread_id()).add(format, args...);
            return *this;
        }
        ULog &flush();

        void close_log_file();

        template <class T>
        ULog &operator<<(T data) { return add_log("{}", data); }

        ULog &operator<<(ULog &(*op)(ULog &)) {
            return (*op)(*this);
        }

        static ULog &get_instance();

        string get_log_file_name();

        long long get_current_thread_id();

        void set_color(ColorCode code);

        void reset_color();

        ULog &add_logvar_list(char *pszFmt, va_list argptr);

    public:

        bool m_std_out;
        //print file name &line
        CircleBuffer *m_buffer_send;
        bool m_is_running;
        int m_inc_log_size = 100 * 1024 * 1024;

        void stop() { m_is_running = false; }

        int m_log_remain_counts = 5;
        list<string> m_log_list;

        std::thread m_write_thread;

    private:
        ofstream m_file_log;
        static ULog *m_log;
        string m_file_name_template;
        LogFileType m_log_file_type;

    private:

        class CThreadLogBuffer {
        public:
            CThreadLogBuffer(long long llThread);

            ~CThreadLogBuffer();

            bool operator==(CThreadLogBuffer &ThreadLogBuffer);

            bool operator==(long long llThread);

            template <typename... Args>
            void add(const char *format, const Args &...args){
                m_current_log += fmt::format(format, args...);
            }

            bool flush();

        public:
            string m_current_log;
            CircleBuffer *m_circle_buffer;
            long long m_thread_id;
        };

    private:
        std::mutex m_mutex_buffer;

        CThreadLogBuffer &get_log_buffer_by_id(long long thread_id);

        void save_log_file();

    private:
        //
        std::map<long long, CThreadLogBuffer *> m_thread_log_buffer;

    };

    static ULog &flush(ULog &lftLog) {
        return lftLog.flush();
    }

    static ULog &endl(ULog &lftLog) {
        return lftLog << "\r\n" << flush;
    }

    static ULog &date_time(ULog &lftLog) {
        char szTime[32] = {0};

        time_t timeNow = time(NULL);
        strftime(szTime, 32, "%Y%m%d-%H:%M:%S ", localtime(&timeNow));

        return lftLog << szTime;
    }


    extern ULog &kULog;
}
//color define
//cout << "\033[1;31mbold red text\033[0m\n";
//foreground background
//black        30         40
//red          31         41
//green        32         42
//yellow       33         43
//blue         34         44
//magenta      35         45
//cyan         36         46
//white        37         47
//Additionally, you can use these:
//
//reset             0  (everything back to normal)
//bold/bright       1  (often a brighter shade of the same colour)
//underline         4
//inverse           7  (swap foreground and background colours)
//bold/bright off  21
//underline off    24
//inverse off      27

#endif //LOLOG_LOG_H
