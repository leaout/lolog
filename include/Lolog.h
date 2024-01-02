/**
* Created by chenly on 1/2/24.
* Description
**/


#ifndef LOLOG_LOLOG_H
#define LOLOG_LOLOG_H

#include <string>
#include <sstream>

#ifdef __linux
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
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


    void debug(const char *fmt, ...);
    void info(const char *fmt, ...);
    void warn(const char *fmt, ...);
    void error(const char *fmt, ...);
    void fatal(const char *fmt, ...);

    void debugex(const char *file_name, int line, const char *fmt, ...);
    void infoex(const char *file_name, int line, const char *fmt, ...);
    void warnex(const char *file_name, int line, const char *fmt, ...);
    void errorex(const char *file_name, int line, const char *fmt, ...);
    void fatalex(const char *file_name, int line, const char *fmt, ...);

#define DEBUGEX(param, args...) debugex(__FILENAME__,__LINE__, param, ##args)
#define INFOEX(param, args...) infoex(__FILENAME__,__LINE__, param, ##args)
#define WARNEX(param, args...) warnex(__FILENAME__,__LINE__, param, ##args)
#define ERROREX(param, args...) errorex(__FILENAME__,__LINE__, param, ##args)
#define FATALEX(param, args...) fatalex(__FILENAME__,__LINE__, param, ##args)

    class LogMessage {
    public:
        LogMessage(const char *file, int line, int log_level);
        ~LogMessage();
        std::stringstream &stream() { return m_stream; }
    private:
        // The real data is cached thread-locally.
        std::stringstream m_stream;
        int m_log_level = 0;
    };

    class LogMessageVoidify {
    public:
        LogMessageVoidify() {}
        // This has to be an operator with a precedence lower than << but
        // higher than ?:
        void operator&(std::ostream &os) {}
    };
#define COMPACT_LOG_EX(ClassName, severity, ...)  \
    ClassName(__FILE__, __LINE__,severity            \
     ,##__VA_ARGS__)
#define LOG_STREAM(severity) COMPACT_LOG_EX(LogMessage,severity).stream()
#define LAZY_STREAM(stream)                            \
            LogMessageVoidify() & (stream)
#define LODEBUG()   LAZY_STREAM(LOG_STREAM(4))
#define LOINFO()    LAZY_STREAM(LOG_STREAM(3))
#define LOWARN()    LAZY_STREAM(LOG_STREAM(2))
#define LOERROR()   LAZY_STREAM(LOG_STREAM(1))
#define LOFATAL()   LAZY_STREAM(LOG_STREAM(0))
}



#endif //LOLOG_LOLOG_H
