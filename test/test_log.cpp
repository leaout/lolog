/**
* Copyright (c) 2024 chenly. All rights reserved.
* Created by chenly on 1/2/24.
* Description
**/
#include <thread>
#include "gtest/gtest.h"
#include "../include/Lolog.h"


using namespace std;

class LologTest : public testing::Test {
public:
    ~LologTest() {}

protected:

    virtual void SetUp() {
        lolog::init_logging("test.log",5,200<<20);
        lolog::set_log_level("Debug");
    }

    virtual void TearDown() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
};

TEST_F(LologTest, print_log) {
    using namespace lolog;
    debug("this is debug");

    LODEBUG("this is macro debug");

    info("this is info");
    LOINFO("this is macro info");

    warn("this is warn");
    LOWARN("this is macro warn");

    error("this is error");

    LOERROR("this is macro error");

    fatal("this is fatal");
    LOFATAL("this is macro fatal");
}

TEST_F(LologTest, batct_log) {
    int print_cout = 1000;
    using namespace lolog;
    std::thread th1([=]() {
        for (int i = 0; i < print_cout; ++i) {
            LOINFO("UINFO---------testtesttesttesttesttesttesttest--------------------------:{}",i);
            lolog::info("UINFO-----------------------------------");
        }
    });
    std::thread th2([=]() {
        for (int i = 0; i < print_cout; ++i) {
            LODEBUG("UDEBUG-----------------------------------{}",i);
            lolog::debug("UDEBUG-----------------------------------");
        }

    });
    std::thread th3([=]() {
        for (int i = 0; i < print_cout; ++i) {
            LOERROR("UERROR-----------------------------------{}",i);
            lolog::error("UERROR-----------------------------------");
        }

    });
    std::thread th4([=]() {
        for (int i = 0; i < print_cout; ++i) {
            LOFATAL("UFATAL--------------------------------------{}",i);
            lolog::fatal("UFATAL-----------------------------------");
        }

    });
    
    th1.join();
    th2.join();
    th3.join();
    th4.join();
}

