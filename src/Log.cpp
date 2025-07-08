#include "Log.h"
#include <string.h>
#include <stdarg.h>
#include <functional>
#include "Msg.h"

namespace lolog {
    ULog *ULog::m_log = nullptr;

    ULog &kULog = ULog::get_instance();

    ULog &ULog::get_instance() {
        if (m_log == nullptr)
            m_log = new ULog;

        return *m_log;
    }

    ULog::ULog() {
        m_is_running = false;

        m_std_out = false;

        m_log_file_type = LogFileType::DateTime;

        m_buffer_send = nullptr;
    }

    ULog::~ULog() {
        m_is_running = false;

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        if (m_write_thread.joinable())
                m_write_thread.join();
                
        for (auto &it : m_thread_log_buffer) {
            delete (it.second);
        }

        m_log = nullptr;
    }


    void ULog::set_log_file_type(LogFileType nLogFileType/*=TyLogType_DateTime*/ ) {
        m_log_file_type = nLogFileType;
    }


    string ULog::get_log_file_name() {
        static int log_sequence_no = 0;

        string strLogFileName = m_file_name_template;
        switch (m_log_file_type) {
            case LogFileType::DateTime: {
                char szTime[32] = {0};
                time_t timeNow = time(nullptr);

                strftime(szTime, 32, "-%Y%m%d-%H%M%S.log", localtime(&timeNow));
                strLogFileName += szTime;
            }
                break;

            case LogFileType::Date: {
                char szTime[32] = {0};
                time_t timeNow = time(nullptr);

                strftime(szTime, 32, "-%Y%m%d.log", localtime(&timeNow));
                strLogFileName += szTime;
            }
                break;

            default: {
                char szSn[32] = {0};
                sprintf(szSn, "-%02d.log", log_sequence_no);
                strLogFileName += szSn;

                if (++log_sequence_no > 50)
                    log_sequence_no = 0;
            }
                break;
        }
        return strLogFileName;
    }

    long long ULog::get_current_thread_id() {
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

    bool ULog::open_log_file(const char *log_path_name) {
        static bool bIsRunThread = true;
        m_is_running = true;

        m_file_name_template = log_path_name;
        if (m_file_name_template.compare(m_file_name_template.size() - 4, 4, ".log") == 0)
            m_file_name_template.erase(m_file_name_template.size() - 4, 4);

        close_log_file();
        if (bIsRunThread) {

            std::thread th(bind(&ULog::save_log_file, this));
            th.swap(m_write_thread);
            bIsRunThread = false;
        }
        auto path = get_path(m_file_name_template);
#ifdef __linux
        //remove old log when restarting
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


    void ULog::close_log_file() {
        if (m_file_log.is_open())
            m_file_log.close();
    }

    ULog &ULog::flush() {
        CThreadLogBuffer &ThreadLogBuffer = get_log_buffer_by_id(get_current_thread_id());
        ThreadLogBuffer.flush();

        return *this;
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
        MsgDef * pMsg = (MsgDef * )new char[1024 * 500];
        LyMsg msg_head;

        int nLogCount = 0;

        map<long long, bool> large_log_mark;

        while (m_is_running && (m_log != nullptr)) {
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
                        //print scream
                        int nDataBufferLen = pCircleBuffer->get_used_buff_len();
                        if (nDataBufferLen <= 1024 * 100) {
                            fwrite(pMsg->data, sizeof(char), pMsg->len, stderr);
                            large_log_mark[log_buffer.second->m_thread_id] = false;
                        } else {
                            if (!large_log_mark[log_buffer.second->m_thread_id])
                                fprintf(stderr, "thread[%lld] log buffer size[%d] > 100K, do not write it on screen.",
                                        log_buffer.second->m_thread_id, nDataBufferLen);

                            large_log_mark[log_buffer.second->m_thread_id] = true;
                        }
                    }

                    if ((m_log_file_type == LogFileType::AutoIncSn ||
                         m_log_file_type == LogFileType::DateTime)
                        && (m_file_log.tellp() > m_inc_log_size)) {
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
        string color = "\033[" + to_string((int) code) + "m";
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

    bool ULog::CThreadLogBuffer::flush() {
        if (m_current_log.empty())
            return true;

        LyMsg msg;
        if (msg.write(m_current_log.c_str(), 999, m_current_log.size(), m_circle_buffer)) {
            m_current_log.erase();
            return true;
        }

        return false;
    }

    bool ULog::CThreadLogBuffer::operator==(CThreadLogBuffer &ThreadLogBuffer) {
        return this->m_thread_id == ThreadLogBuffer.m_thread_id;
    }

    bool ULog::CThreadLogBuffer::operator==(long long llThread) {
        return this->m_thread_id == llThread;
    }
}