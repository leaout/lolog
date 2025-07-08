//
// Created by chenly on 9/2/21.
//
#include <iostream>
#include <iomanip>
#include <omp.h>
#include <vector>
#include <string_view>
#include <cstring>
#include <atomic>
#include <chrono>
#include <memory>
// #include "include/Lolog.h"
#include "include/Lolog-Inl.h"

using namespace std;
// using namespace lolog;

class BenchMark {
public:
    BenchMark() {
        init();
    }

    void init() {
        lolog::init_logging("test.log", 5, 200 << 20);
        lolog::set_log_level("Info");
    }

    ~BenchMark() {

    }

public:

};

BenchMark* bench_mark = nullptr;


#if defined(__linux)

static std::string_view TrimSpace(std::string_view s) {
    size_t start = 0;
    while (start < s.size() && isspace(s[start])) {
        start++;
    }
    size_t limit = s.size();
    while (limit > start && isspace(s[limit - 1])) {
        limit--;
    }
    return std::string_view(s.data() + start, limit - start);
}

#endif

//BENCHMARK_MAIN();
void print_environment() {
    std::fprintf(stderr, "Lolog:    version %d.%d\n", 1,
                 2);

#if defined(__linux)
    time_t now = time(nullptr);
    std::fprintf(stderr, "Date:       %s",
                 ctime(&now));  // ctime() adds newline

    FILE *cpuinfo = std::fopen("/proc/cpuinfo", "r");
    if (cpuinfo != nullptr) {
        char line[1000];
        int num_cpus = 0;
        std::string cpu_type;
        std::string cache_size;
        while (fgets(line, sizeof(line), cpuinfo) != nullptr) {
            const char *sep = strchr(line, ':');
            if (sep == nullptr) {
                continue;
            }
            std::string_view key = TrimSpace(std::string_view(line, sep - 1 - line));
            std::string_view val = TrimSpace(std::string_view(sep + 1));
            if (key == "model name") {
                ++num_cpus;
                cpu_type = val;
            } else if (key == "cache size") {
                cache_size = val;
            }
        }
        std::fclose(cpuinfo);
        std::fprintf(stderr, "CPU:        %d * %s\n", num_cpus, cpu_type.c_str());
        std::fprintf(stderr, "CPUCache:   %s\n", cache_size.c_str());
    }
#endif
}

const size_t kMax = 100 * 10000;

class Stat {
public:
    void finished_single_op(int counter = 1) {
        m_done+=counter;
        if (m_done >= m_next_report) {
            if (m_next_report < 1000)
                m_next_report += 100;
            else if (m_next_report < 5000)
                m_next_report += 500;
            else if (m_next_report < 10000)
                m_next_report += 1000;
            else if (m_next_report < 50000)
                m_next_report += 5000;
            else if (m_next_report < 100000)
                m_next_report += 10000;
            else if (m_next_report < 500000)
                m_next_report += 50000;
            else
                m_next_report += 100000;
            int ret = m_done.load();
            std::fprintf(stderr, "... finished %d ops%30s\r", ret, "");
            std::fflush(stderr);
        }
    }

public:

    std::atomic<int> m_done = 0;
    int m_next_report = 100;
};

void print_result(const char* func_name, int time_cost,Stat& stat){
    cout.setf(ios::left);
    std::cout << std::setw(32) <<func_name  << " : " << stat.m_done / (double)time_cost * 1000*1000 << " op/s"<< " total:" << stat.m_done << " timecost:" << time_cost << " μs"<<std::endl;
}

typedef void (*func)();
vector<func> bench_funcs;

#define BENCH_MARK(fn) \
        bench_funcs.push_back(fn);


void bench_lolog(){
    Stat stat;
    LOINFO() << "test begin";
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 100000; ++i){
        lolog::info("this is a info log ");
        stat.finished_single_op();
    }

    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    int time_cost = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

    print_result(__FUNCTION__,time_cost, stat);
}


int main(int argc, char *argv[]) {

    auto bench_shared = make_shared<BenchMark>();
    bench_mark = bench_shared.get();
    print_environment();

    BENCH_MARK(bench_lolog);

    for(auto&fn : bench_funcs){
        fn();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    return 0;
}