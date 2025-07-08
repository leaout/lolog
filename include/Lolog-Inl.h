/**
 * Created by chenly on 1/2/24.
 * Description
 **/
#pragma once

#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <cstring>
#include <fstream>
#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>

namespace lolog {
using namespace std;

class CircleBuffer {
public:
    CircleBuffer() {
        m_base_address = nullptr;
        m_buffer_size = 0;
        memset(m_buffer_name, 0, sizeof(m_buffer_name));
        free();
    }

    CircleBuffer(size_t size, const char *buff_name) {
        m_base_address = nullptr;
        m_buffer_size = 0;
        alloc(size, buff_name);
    }

    ~CircleBuffer() { free(); }

    bool alloc(size_t size, const char *buff_name) {
        strcpy(m_buffer_name, buff_name);

        if (m_buffer_size != size) {
            free();
            m_buffer_size = size;
            m_base_address = new char[m_buffer_size + 32];
            m_tail_address = m_base_address + m_buffer_size + 1;
        }

        reset();

        return (m_base_address != NULL);
    }

    void free() {
        delete[] m_base_address;

        m_base_address = NULL;
        m_tail_address = NULL;
        m_buffer_size = 0;

        reset();
    }

    void reset() {
        if (m_base_address != NULL) {
            m_head = m_base_address;
            m_tail = m_base_address;
        } else {
            m_head = NULL;
            m_tail = NULL;
        }

        m_is_writing_data = false;
        m_is_reading_data = false;
    }

    size_t get_max_len() { return m_buffer_size; }

    size_t get_used_buff_len() {
        char *head = (m_head < m_tail_address) ? (char *)m_head : m_base_address;
        char *tail = (m_tail < m_tail_address) ? (char *)m_tail : m_base_address;

        char *vtail = (tail >= head) ? tail : (tail + m_buffer_size + 1);
        return size_t(vtail - head);
    }

    size_t get_available_buff_len() {
        char *head = (m_head < m_tail_address) ? (char *)m_head : m_base_address;
        char *tail = (m_tail < m_tail_address) ? (char *)m_tail : m_base_address;

        if (head == tail) return m_buffer_size;

        char *vtail = (tail >= head) ? tail : (tail + m_buffer_size + 1);
        return m_buffer_size - (vtail - head);
    }

    size_t get_continuous_available_buff_len() {
        char *head = (m_head < m_tail_address) ? (char *)m_head : m_base_address;
        char *tail = (m_tail < m_tail_address) ? (char *)m_tail : m_base_address;

        if (head > tail) return (head - tail) - 1;

        if ((tail == head) && (head == m_base_address)) return m_buffer_size;

        return size_t(m_tail_address - tail);
    }

    size_t get_continuous_data_buff_len() {
        char *head = (m_head < m_tail_address) ? (char *)m_head : m_base_address;
        char *tail = (m_tail < m_tail_address) ? (char *)m_tail : m_base_address;

        if (tail >= head) return tail - head;

        return size_t(m_tail_address - head);
    }

    bool is_continuous_data_len(size_t count) { return m_head + count < m_tail_address; }

    size_t read(char *dest, long count) {
        if (get_used_buff_len() < count) return 0;

        if (m_is_reading_data || count <= 0) return 0;

        char *head = (m_head < m_tail_address) ? (char *)m_head : m_base_address;
        char *tail = (m_tail < m_tail_address) ? (char *)m_tail : m_base_address;

        if (head == tail) return 0;

        m_is_reading_data = true;

        size_t copy_len = 0;
        if (head < tail) {
            while (head < tail && count--) {
                *dest++ = *head++;
                ++copy_len;
            }
        } else {
            while (head < m_tail_address && count--) {
                *dest++ = *head++;
                ++copy_len;
            }

            if (count > 0) {
                head = m_base_address;
                while (head < tail && count--) {
                    *dest++ = *head++;
                    ++copy_len;
                }
            }
        }

        m_head = head;
        m_is_reading_data = false;
        return copy_len;
    }

    bool write(const char *src, long count) {
        if (m_is_writing_data || count <= 0) return false;

        if (count > get_available_buff_len()) return false;

        m_is_writing_data = true;

        char *tail = (m_tail < m_tail_address) ? (char *)m_tail : m_base_address;

        if (tail + count < m_tail_address) {
            while (count--) *tail++ = *src++;
        } else {
            while (tail < m_tail_address && count--) *tail++ = *src++;

            if (count > 0) {
                tail = m_base_address;
                while (count--) *tail++ = *src++;
            }
        }

        m_tail = tail;

        m_is_writing_data = false;
        return true;
    }

    char *get_read_position() { return (char *)m_head; }

    char *get_write_position() { return (char *)m_tail; }

    bool inc_data_len(long count) {
        if (count <= 0) return false;

        m_tail += count;
        if (m_tail >= m_tail_address) m_tail -= m_buffer_size + 1;

        return true;
    }

    bool dec_data_len(long count) {
        if (count <= 0) return false;

        size_t data_len = get_used_buff_len();
        if (count > data_len) count = data_len;

        m_head += count;
        if (m_head >= m_tail_address) m_head -= m_buffer_size + 1;

        return true;
    }

    char operator[](size_t index) {
        if ((m_head + index) >= m_tail_address) return m_base_address[index - (m_tail_address - m_head)];

        return m_head[index];
    }

    int memcmp(const void *buf, long count, size_t offset = 0) {
        if (!count) return (0);

        char *buf1 = (m_head < m_tail_address) ? (char *)m_head : m_base_address;
        char *buf2 = (char *)buf;

        if (offset) {
            buf1 += offset;
            if (buf1 >= m_tail_address) buf1 -= m_buffer_size + 1;
        }

        bool more_cmp = false;
        while ((buf1 < m_tail_address) && --count && (more_cmp = (*buf1 == *buf2))) {
            ++buf1;
            ++buf2;
        }

        if (more_cmp && (count > 0)) {
            buf1 = m_base_address;

            while (--count && (*buf1 == *buf2)) {
                ++buf1;
                ++buf2;
            }
        }

        return (*((unsigned char *)buf1) - *((unsigned char *)buf2));
    }

    void *memcpy(void *dst, long count, size_t offset = 0) {
        char *src = (char *)(m_head < m_tail_address) ? (char *)m_head : m_base_address;
        char *dest = (char *)dst;

        if (offset) {
            src += offset;
            if (src >= m_tail_address) src -= m_buffer_size + 1;
        }

        while (src < m_tail_address && count--) *dest++ = *src++;

        if (count > 0) {
            src = m_base_address;
            while (src < m_tail_address && count--) *dest++ = *src++;
        }

        return (dst);
    }

    size_t strstr(const char *str) {
        size_t nSearchStrLen = strlen(str);
        size_t nDataStrLen = get_used_buff_len();
        if (nDataStrLen < nSearchStrLen) return -1;

        size_t nSearchCount = nDataStrLen - nSearchStrLen;
        for (size_t i = 0; i <= nSearchCount; ++i) {
            if (memcmp(str, nSearchStrLen, i) == 0) return i;
        }

        return -1;
    }

public:
    char m_buffer_name[64];

protected:
    char *m_base_address;
    char *m_tail_address;
    size_t m_buffer_size;

    volatile char *m_head;
    volatile char *m_tail;

    volatile bool m_is_writing_data;
    volatile bool m_is_reading_data;
};

#pragma pack(4)
struct MsgDef {
    unsigned int flag;  // msg head flag 0xFFFFFFFF
    int type;           // msg type
    int len;            // msg len
    char data[0];
};
#pragma pack()

class LyMsg {
public:
    LyMsg() {
        m_using_tail = false;
        m_check_msg_version = true;
    }

    ~LyMsg() {}

    bool read(CircleBuffer *circle_buffer, MsgDef *msg, size_t max_expected_len) {
        size_t nDataLen = 0;
        const int head_len = sizeof(MsgDef);

        nDataLen = circle_buffer->get_used_buff_len();
        if (nDataLen <= head_len) {
            msg->flag = 0;
            msg->type = 0;
            msg->len = 0;

            return false;
        }

        if (0 == circle_buffer->memcmp(&MSG_TAG, sizeof(int))) {
            circle_buffer->memcpy((char *)msg, head_len);
            if (msg->len <= 0 || msg->len > max_expected_len - head_len) {
                circle_buffer->dec_data_len(1);
                return false;
            }

            if (circle_buffer->get_used_buff_len() < head_len + msg->len) {
                return false;
            }

            if (m_check_msg_version) {
                m_using_tail = is_advanced_msg(circle_buffer, msg);
                m_check_msg_version = false;
            }

            if (m_using_tail && !is_advanced_msg(circle_buffer, msg)) {
                msg->flag = MSG_TAG;
                msg->type = 0;
                msg->len = abandon_error_data(circle_buffer);
                return false;
            }

            circle_buffer->read((char *)msg, head_len + msg->len);

            if (m_using_tail) msg->len -= sizeof(int);

            return true;
        } else {
            msg->flag = MSG_TAG;
            msg->type = 0;
            msg->len = abandon_error_data(circle_buffer);
        }

        return false;
    }

    bool write(const char *data, int type, size_t count, CircleBuffer *circle_buffer) {
        MsgDef msg_head = {MSG_TAG, type, int(count + sizeof(int))};

        if (circle_buffer->get_available_buff_len() < msg_head.len + sizeof(MsgDef)) return false;

        circle_buffer->write((char *)&msg_head, sizeof(MsgDef));
        circle_buffer->write(data, msg_head.len - sizeof(int));
        int nTail = get_tail_value(msg_head.len);
        circle_buffer->write((char *)&nTail, sizeof(int));

        return true;
    }

    size_t abandon_error_data(CircleBuffer *circle_buffer) {
        char data[1024] = {0};
        char *p = data;
        size_t error_len = 0;

        size_t all_data_len = circle_buffer->get_used_buff_len();
        if (all_data_len > 1024) all_data_len = 1024;

        size_t data_len = 0;
        do {
            if (error_len < 1000) {
                circle_buffer->memcpy(p, 1);
                ++p;
                ++error_len;
            } else {
                break;
            }

            circle_buffer->dec_data_len(1);
            data_len = circle_buffer->get_used_buff_len();
        } while (data_len > 3 && 0 != circle_buffer->memcmp(&MSG_TAG, sizeof(int)));

        return error_len;
    }

protected:
    inline static int get_tail_value(int msg_len) {
        return msg_len ^ 0xaaaaaaaa;  // ff get oppset value
    }

private:
    static bool is_advanced_msg(MsgDef *pmsg) { return *(int *)(pmsg->data + pmsg->len - sizeof(int)) == get_tail_value(pmsg->len); }

    static bool is_advanced_msg(CircleBuffer *circle_buffer, MsgDef *pmsg) {
        int tail = get_tail_value(pmsg->len);
        int tail_pos = sizeof(MsgDef) + pmsg->len - sizeof(int);
        return (circle_buffer->memcmp((char *)&tail, sizeof(int), tail_pos) == 0);
    }

    int discard_error_data(CircleBuffer *circle_buffer) {
        char data[1024] = {0};
        char *p = data;
        int error_len = 0;

        size_t all_data_len = circle_buffer->get_used_buff_len();
        if (all_data_len > 1024) all_data_len = 1024;

        size_t data_len = 0;
        do {
            if (error_len < 1000) {
                circle_buffer->memcpy(p, 1);
                ++p;
                ++error_len;
            } else {
                break;
            }

            circle_buffer->dec_data_len(1);
            data_len = circle_buffer->get_used_buff_len();
        } while (data_len > 3 && 0 != circle_buffer->memcmp(&MSG_TAG, sizeof(int)));

        return error_len;
    }

public:
    unsigned int MSG_TAG = 0xFFFFFFFF;

protected:
    bool m_using_tail;
    bool m_check_msg_version;
};

#define LIST_OF_LOGLEVEL \
    X(Fatal)             \
    X(Error)             \
    X(Warn)              \
    X(Info)              \
    X(Debug)

#define X(name) name,
enum class LogLevel { LIST_OF_LOGLEVEL };
#undef X

static std::map<std::string, enum LogLevel> kLevelMap;

enum class LogFileType { DateTime = 0, Date = 1, AutoIncSn = 2 };

template <typename E>
constexpr auto toUType(E enumerator) noexcept {
    return static_cast<std::underlying_type_t<E>>(enumerator);
}

std::string exec(const char *cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

string get_path(const string full_file_path) {
    std::string::size_type npos = full_file_path.rfind("/");
    if (npos == std::string::npos) {
        return "./";

    } else {
        return full_file_path.substr(0, npos + 1);
    }
}

class ULog {
public:
    enum class ColorCode { BLACK = 30, RED = 31, GREEN = 32, YELLOW = 33, BLUE = 34, MAGENTA = 35, CYAN = 36, WHITE = 37 };

public:
    ULog() {
        m_is_running = false;

        m_std_out = false;

        m_log_file_type = LogFileType::DateTime;

        m_buffer_send = nullptr;
    }

    ~ULog() {
        m_is_running = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        if (m_write_thread.joinable())
                m_write_thread.join();
        for (auto &it : m_thread_log_buffer)
        {
            delete (it.second);
        }
        // m_log = nullptr;
    }

    void set_log_file_type(LogFileType nLogFileType /*=TyLogType_DateTime*/) { m_log_file_type = nLogFileType; }

    string get_log_file_name() {
        static int log_sequence_no = 0;

        string strLogFileName = m_file_name_template;
        switch (m_log_file_type) {
            case LogFileType::DateTime: {
                char szTime[32] = {0};
                time_t timeNow = time(nullptr);

                strftime(szTime, 32, "-%Y%m%d-%H%M%S.log", localtime(&timeNow));
                strLogFileName += szTime;
            } break;

            case LogFileType::Date: {
                char szTime[32] = {0};
                time_t timeNow = time(nullptr);

                strftime(szTime, 32, "-%Y%m%d.log", localtime(&timeNow));
                strLogFileName += szTime;
            } break;

            default: {
                char szSn[32] = {0};
                sprintf(szSn, "-%02d.log", log_sequence_no);
                strLogFileName += szSn;

                if (++log_sequence_no > 50) log_sequence_no = 0;
            } break;
        }
        return strLogFileName;
    }

    long long get_current_thread_id() {
        long long tid = 0;
#ifdef _WIN32
#include <process.h>
        tid = GetCurrentThreadId();
#else
#include <pthread.h>
        tid = pthread_self();
#endif
        return tid;
    }

    bool open_log_file(const char *log_path_name) {
        static bool bIsRunThread = true;
        m_is_running = true;

        m_file_name_template = log_path_name;
        if (m_file_name_template.compare(m_file_name_template.size() - 4, 4, ".log") == 0) m_file_name_template.erase(m_file_name_template.size() - 4, 4);

        close_log_file();
        if (bIsRunThread) {
            std::thread th(bind(&ULog::save_log_file, this));
            th.swap(m_write_thread);
            bIsRunThread = false;
        }
        auto path = get_path(m_file_name_template);
#ifdef __linux
        // remove old log when restarting
        string cmd = "cd " + path + " && rm -f `ls -t *.log | awk 'NR>" + to_string(m_log_remain_counts) + "'`";
        try {
            exec(cmd.data());
        } catch (exception e) {
        }
#endif
        locale::global(locale(""));
        string log_file_name = get_log_file_name();
        if (m_log_file_type == LogFileType::Date)
            m_file_log.open(log_file_name.c_str(), ios::binary | ios::app);
        else
            m_file_log.open(log_file_name.c_str(), ios::binary | ios::trunc);

        m_log_list.emplace_back(log_file_name);

        locale::global(locale("C"));
        return m_file_log.is_open();
    }

    void close_log_file() {
        if (m_file_log.is_open()) m_file_log.close();
    }

    ULog &add_log(const char *pszFmt, ...) {
        va_list argptr;

        va_start(argptr, pszFmt);
        add_logvar_list((char *)pszFmt, argptr);
        va_end(argptr);

        return *this;
    }

    ULog &add_logvar_list(char *pszFmt, va_list argptr) {
        CThreadLogBuffer &thread_log_buffer = get_log_buffer_by_id(get_current_thread_id());
        thread_log_buffer.add(pszFmt, argptr);

        return *this;
    }

    ULog &operator<<(char *pszData) { return add_log("%s", pszData); }

    ULog &operator<<(const char *pszData) { return add_log("%s", pszData); }

    ULog &operator<<(string &strData) { return add_log("%s", strData.c_str()); }

    ULog &operator<<(unsigned int nData) { return add_log("%d", nData); }

    ULog &operator<<(unsigned long nData) { return add_log("%ld", nData); }

    ULog &operator<<(char cData) { return add_log("%c", cData); }

    ULog &operator<<(int nData) { return add_log("%d", nData); }

    ULog &operator<<(long nData) { return add_log("%ld", nData); }

    ULog &operator<<(long long nData) { return add_log("%lld", nData); }

    ULog &operator<<(float fData) { return add_log("%f", fData); }

    ULog &operator<<(double fData) { return add_log("%f", fData); }

    ULog &flush() {
        CThreadLogBuffer &ThreadLogBuffer = get_log_buffer_by_id(get_current_thread_id());
        ThreadLogBuffer.flush();

        return *this;
    }

    ULog &operator<<(ULog &(*op)(ULog &)) { return (*op)(*this); }

    static ULog &get_instance();

    void set_color(ColorCode code);

    void reset_color();

public:
    bool m_std_out;
    // print file name &line
    CircleBuffer *m_buffer_send;
    bool m_is_running;
    int m_inc_log_size = 100 * 1024 * 1024;

    void stop() { m_is_running = false; }

    int m_log_remain_counts = 5;
    list<string> m_log_list;
    
    std::thread m_write_thread;

private:
    ofstream m_file_log;
    // ULog *m_log = nullptr;
    string m_file_name_template;
    LogFileType m_log_file_type;

private:
    class CThreadLogBuffer {
    public:
        CThreadLogBuffer(long long llThread);

        ~CThreadLogBuffer();

        bool operator==(CThreadLogBuffer &ThreadLogBuffer);

        bool operator==(long long llThread);

        void add(const char *pszFmt, va_list argptr);

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

static ULog &flush(ULog &lftLog) { return lftLog.flush(); }

static ULog &endl(ULog &lftLog) { return lftLog << "\r\n" << flush; }

static ULog &date_time(ULog &lftLog) {
    char szTime[32] = {0};

    time_t timeNow = time(NULL);
    strftime(szTime, 32, "%Y%m%d-%H:%M:%S ", localtime(&timeNow));

    return lftLog << szTime;
}

static ULog &pid(ULog &lftLog) {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string stid = oss.str();
    return lftLog << std::stoul(stid);
}

extern ULog &kULog;

ULog &kULog = ULog::get_instance();

ULog &ULog::get_instance() {
    static ULog instance;

    return instance;
}

ULog::CThreadLogBuffer &ULog::get_log_buffer_by_id(long long thread_id) {
    auto find_it = m_thread_log_buffer.find(thread_id);
    if (find_it != m_thread_log_buffer.end()) {
        return *(find_it->second);
    }
    std::lock_guard<mutex> lock(m_mutex_buffer);
    auto log_buffer_ptr = new CThreadLogBuffer(thread_id);
    m_thread_log_buffer.insert(std::make_pair(thread_id, log_buffer_ptr));

    return *(log_buffer_ptr);
}

void ULog::save_log_file() {
    MsgDef *pMsg = (MsgDef *)new char[1024 * 500];
    LyMsg msg_head;

    int nLogCount = 0;

    map<long long, bool> large_log_mark;

    while (m_is_running) {
        if (m_thread_log_buffer.empty()) {
            std::this_thread::sleep_for(chrono::microseconds(100));
            continue;
        }

        nLogCount = 0;

        for (auto log_buffer : m_thread_log_buffer) {
            CircleBuffer *pCircleBuffer = log_buffer.second->m_circle_buffer;

            if (msg_head.read(pCircleBuffer, pMsg, 1024 * 400)) {
                if (m_buffer_send) {
                    msg_head.write(pMsg->data, pMsg->type, pMsg->len, m_buffer_send);
                }

                pMsg->data[pMsg->len] = '\0';

                ++nLogCount;
                if (m_file_log.is_open()) {
                    m_file_log.write(pMsg->data, pMsg->len);
                    m_file_log.flush();
                }

                if (m_std_out) {
                    // print scream
                    int nDataBufferLen = pCircleBuffer->get_used_buff_len();
                    if (nDataBufferLen <= 1024 * 100) {
                        fwrite(pMsg->data, sizeof(char), pMsg->len, stderr);
                        large_log_mark[log_buffer.second->m_thread_id] = false;
                    } else {
                        if (!large_log_mark[log_buffer.second->m_thread_id])
                            fprintf(stderr, "thread[%lld] log buffer size[%d] > 100K, do not write it on screen.", log_buffer.second->m_thread_id, nDataBufferLen);

                        large_log_mark[log_buffer.second->m_thread_id] = true;
                    }
                }

                if ((m_log_file_type == LogFileType::AutoIncSn || m_log_file_type == LogFileType::DateTime) && (m_file_log.tellp() > m_inc_log_size)) {
                    m_file_log.close();
                    if (m_log_list.size() >= m_log_remain_counts) {
                        remove(m_log_list.front().c_str());
                        m_log_list.pop_front();
                    }
                    string log_file_name = get_log_file_name();
                    m_file_log.open(log_file_name.c_str(), ios::binary | ios::trunc);
                    m_log_list.emplace_back(log_file_name);
                }
            }
        }

        if (nLogCount == 0) {
            std::this_thread::sleep_for(chrono::microseconds(100));
        }
    }
}

void ULog::set_color(ColorCode code) {
    string color = "\033[" + to_string((int)code) + "m";
    *this << color;
}

void ULog::reset_color() {
    string color = "\033[0m";
    *this << color;
}

//////////////////////////////////////////////////////////////////////////

ULog::CThreadLogBuffer::CThreadLogBuffer(long long llThread) {
    m_thread_id = llThread;
    m_circle_buffer = new CircleBuffer(1024 * 1024 * 5, "thread_buffer");
}

ULog::CThreadLogBuffer::~CThreadLogBuffer() {
    delete m_circle_buffer;
    m_circle_buffer = nullptr;
}

void ULog::CThreadLogBuffer::add(const char *pszFmt, va_list argptr) {
    if (pszFmt != nullptr) {
        char szBuf[10240] = {0};
        int ret = vsnprintf(szBuf, 10240, pszFmt, argptr);
        int size = ret > 10240 ? 10240 : ret;
        m_current_log.append(szBuf, size);
        //    m_strcurrent_log += szBuf;
    }
}

bool ULog::CThreadLogBuffer::flush() {
    if (m_current_log.empty()) return true;

    LyMsg msg;
    if (msg.write(m_current_log.c_str(), 999, m_current_log.size(), m_circle_buffer)) {
        m_current_log.erase();
        return true;
    }

    return false;
}

bool ULog::CThreadLogBuffer::operator==(CThreadLogBuffer &ThreadLogBuffer) { return this->m_thread_id == ThreadLogBuffer.m_thread_id; }

bool ULog::CThreadLogBuffer::operator==(long long llThread) { return this->m_thread_id == llThread; }

char g_log_level_def[][32] = {"[fatal] ", "[error] ", "[warn] ", "[info] ", "[debug] "};

LogLevel g_log_level = LogLevel::Error;
bool g_color = false;
bool g_print_line = false;

inline void init_logging(const char *log_path_name, int log_remain_counts, int log_file_size) {
    kULog.open_log_file(log_path_name);

    g_color = false;

    kULog.m_std_out = false;

    kULog.m_inc_log_size = log_file_size;

    kULog.m_log_remain_counts = log_remain_counts;

#define X(name) kLevelMap[#name] = LogLevel::name;
    LIST_OF_LOGLEVEL
#undef X
}

inline void set_color(bool set) { g_color = set; }

inline void set_std_out(bool set) { kULog.m_std_out = set; }

inline void set_print_file_line(bool set) { g_print_line = set; }

inline void set_log_level(int set) { g_log_level = (LogLevel)set; }

inline void set_log_level(std::string level) {
    auto find_ret = kLevelMap.find(level);
    if (find_ret != kLevelMap.end()) {
        g_log_level = find_ret->second;
    }
}

inline void set_log_formate(int formate) { kULog.set_log_file_type((LogFileType)formate); }

inline void debugex(const char *file_name, int line, const char *fmt, ...) {
    if (g_log_level < LogLevel::Debug) return;

    if (g_color) {
        kULog.set_color(ULog::ColorCode::BLUE);
    }

    kULog << date_time << g_log_level_def[toUType(LogLevel::Debug)];

    kULog << "[" << file_name << ":" << line << "] ";

    va_list argptr;

    va_start(argptr, fmt);
    kULog.add_logvar_list((char *)fmt, argptr);
    va_end(argptr);

    if (g_color) {
        kULog.reset_color();
    }
    kULog << endl;
}

inline void infoex(const char *file_name, int line, const char *fmt, ...) {
    if (g_log_level < LogLevel::Info) return;

    if (g_color) {
        kULog.set_color(ULog::ColorCode::CYAN);
    }

    kULog << date_time << g_log_level_def[toUType(LogLevel::Info)];

    kULog << "[" << file_name << ":" << line << "] ";

    va_list argptr;

    va_start(argptr, fmt);
    kULog.add_logvar_list((char *)fmt, argptr);
    va_end(argptr);

    if (g_color) {
        kULog.reset_color();
    }
    kULog << endl;
}

inline void warnex(const char *file_name, int line, const char *fmt, ...) {
    if (g_log_level < LogLevel::Warn) return;

    if (g_color) {
        kULog.set_color(ULog::ColorCode::YELLOW);
    }

    kULog << date_time << g_log_level_def[toUType(LogLevel::Warn)];

    kULog << "[" << file_name << ":" << line << "] ";

    va_list argptr;

    va_start(argptr, fmt);
    kULog.add_logvar_list((char *)fmt, argptr);
    va_end(argptr);

    if (g_color) {
        kULog.reset_color();
    }
    kULog << endl;
}

inline void errorex(const char *file_name, int line, const char *fmt, ...) {
    if (g_log_level < LogLevel::Error) return;

    if (g_color) {
        kULog.set_color(ULog::ColorCode::RED);
    }

    kULog << date_time << g_log_level_def[toUType(LogLevel::Error)];

    kULog << "[" << file_name << ":" << line << "] ";

    va_list argptr;

    va_start(argptr, fmt);
    kULog.add_logvar_list((char *)fmt, argptr);
    va_end(argptr);

    if (g_color) {
        kULog.reset_color();
    }
    kULog << endl;
}

inline void fatalex(const char *file_name, int line, const char *fmt, ...) {
    if (g_log_level < LogLevel::Fatal) return;

    if (g_color) {
        kULog.set_color(ULog::ColorCode::RED);
    }

    kULog << date_time << g_log_level_def[toUType(LogLevel::Fatal)];

    kULog << "[" << file_name << ":" << line << "] ";

    va_list argptr;

    va_start(argptr, fmt);
    kULog.add_logvar_list((char *)fmt, argptr);
    va_end(argptr);

    if (g_color) {
        kULog.reset_color();
    }
    kULog << endl;
}

inline void debug(const char *fmt, ...) {
    if (g_log_level < LogLevel::Debug) return;

    if (g_color) {
        kULog.set_color(ULog::ColorCode::BLUE);
    }

    kULog << date_time << g_log_level_def[toUType(LogLevel::Debug)];

    va_list argptr;

    va_start(argptr, fmt);
    kULog.add_logvar_list((char *)fmt, argptr);
    va_end(argptr);

    if (g_color) {
        kULog.reset_color();
    }
    kULog << endl;
}

inline void info(const char *fmt, ...) {
    if (g_log_level < LogLevel::Info) return;

    if (g_color) {
        kULog.set_color(ULog::ColorCode::CYAN);
    }

    kULog << date_time << g_log_level_def[toUType(LogLevel::Info)];

    va_list argptr;

    va_start(argptr, fmt);
    kULog.add_logvar_list((char *)fmt, argptr);
    va_end(argptr);

    if (g_color) {
        kULog.reset_color();
    }
    kULog << endl;
}

inline void warn(const char *fmt, ...) {
    if (g_log_level < LogLevel::Warn) return;

    if (g_color) {
        kULog.set_color(ULog::ColorCode::YELLOW);
    }

    kULog << date_time << g_log_level_def[toUType(LogLevel::Warn)];

    va_list argptr;

    va_start(argptr, fmt);
    kULog.add_logvar_list((char *)fmt, argptr);
    va_end(argptr);

    if (g_color) {
        kULog.reset_color();
    }
    kULog << endl;
}

inline void error(const char *fmt, ...) {
    if (g_log_level < LogLevel::Error) return;

    if (g_color) {
        kULog.set_color(ULog::ColorCode::RED);
    }

    kULog << date_time << g_log_level_def[toUType(LogLevel::Error)];

    va_list argptr;

    va_start(argptr, fmt);
    kULog.add_logvar_list((char *)fmt, argptr);
    va_end(argptr);

    if (g_color) {
        kULog.reset_color();
    }
    kULog << endl;
}

inline void fatal(const char *fmt, ...) {
    if (g_log_level < LogLevel::Fatal) return;

    if (g_color) {
        kULog.set_color(ULog::ColorCode::RED);
    }

    kULog << date_time << g_log_level_def[toUType(LogLevel::Fatal)];

    va_list argptr;

    va_start(argptr, fmt);
    kULog.add_logvar_list((char *)fmt, argptr);
    va_end(argptr);

    if (g_color) {
        kULog.reset_color();
    }
    kULog << endl;
}

#ifdef __linux
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#endif

#define DEBUGEX(param, args...) lolog::debugex(__FILENAME__, __LINE__, param, ##args)
#define INFOEX(param, args...) lolog::infoex(__FILENAME__, __LINE__, param, ##args)
#define WARNEX(param, args...) lolog::warnex(__FILENAME__, __LINE__, param, ##args)
#define ERROREX(param, args...) lolog::errorex(__FILENAME__, __LINE__, param, ##args)
#define FATALEX(param, args...) lolog::fatalex(__FILENAME__, __LINE__, param, ##args)

class LogMessage {
public:
    LogMessage(const char *file, int line, int log_level) {
        m_log_level = log_level;
        if ((toUType(g_log_level) >= m_log_level) && g_print_line) m_stream << "[" << file << ":" << line << "] ";
    }
    ~LogMessage() {
        if (toUType(g_log_level) >= m_log_level) {
            switch ((LogLevel)m_log_level) {
                case LogLevel::Debug: {
                    debug("%s", m_stream.str().c_str());
                    break;
                }
                case LogLevel::Info: {
                    info("%s", m_stream.str().c_str());
                    break;
                }
                case LogLevel::Warn: {
                    warn("%s", m_stream.str().c_str());
                    break;
                }
                case LogLevel::Error: {
                    error("%s", m_stream.str().c_str());
                    break;
                }
                case LogLevel::Fatal: {
                    fatal("%s", m_stream.str().c_str());
                    break;
                }
            }
        }
    }
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

} // namespace lolog

#define LOCOMPACT_LOG_EX(ClassName, severity, ...) ClassName(__FILE__, __LINE__, severity, ##__VA_ARGS__)
#define LOLOG_STREAM(severity) LOCOMPACT_LOG_EX(lolog::LogMessage, severity).stream()
#define LOLAZY_STREAM(stream) lolog::LogMessageVoidify() & (stream)
#define LODEBUG() LOLAZY_STREAM(LOLOG_STREAM(4))
#define LOINFO() LOLAZY_STREAM(LOLOG_STREAM(3))
#define LOWARN() LOLAZY_STREAM(LOLOG_STREAM(2))
#define LOERROR() LOLAZY_STREAM(LOLOG_STREAM(1))
#define LOFATAL() LOLAZY_STREAM(LOLOG_STREAM(0))