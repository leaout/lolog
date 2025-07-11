/**
 * Copyright (c) 2024 chenly. All rights reserved.
 * Created by chenly on 1/2/24.
 * Description
 **/
#include <thread>
#include "gtest/gtest.h"
#include "../include/Lolog-Inl.h"

using namespace std;

class LologPrintTest : public testing::Test
{
public:
    ~LologPrintTest() {}

protected:
    virtual void SetUp()
    {
        // lolog::init_logging("test.log", 5, 200 << 20);
        // lolog::set_log_level("Debug");
    }

    virtual void TearDown()
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
};

TEST_F(LologPrintTest, print_log)
{
    lolog::debug("print debug");

    LODEBUG("print macro debug");

    lolog::info("print info");
    LOINFO("print macro info");

    lolog::warn("print warn");
    LOWARN("print macro warn");

    lolog::error("print error");

    LOERROR("print macro error");

    lolog::fatal("print fatal");
    LOFATAL("print macro fatal");
}

TEST_F(LologPrintTest, batct_log)
{
    int print_cout = 10;
    using namespace lolog;
    std::thread th1([=]()
                    {
        for (int i = 0; i < print_cout; ++i) {
            LOINFO("UINFO---------testtesttesttesttesttesttesttest--------------------------:{}",i);
            lolog::info("UINFO-----------------------------------");
        } });
    std::thread th2([=]()
                    {
                        for (int i = 0; i < print_cout; ++i)
                        {
                            LODEBUG("UDEBUG-----------------------------------{}", i);
                            lolog::debug("UDEBUG-----------------------------------");
                        }
                    });
    std::thread th3([=]()
                    {
                        for (int i = 0; i < print_cout; ++i)
                        {
                            LOERROR("UERROR-----------------------------------{}", i);
                            lolog::error("UERROR-----------------------------------");
                        }
                    });
    std::thread th4([=]()
                    {
                        for (int i = 0; i < print_cout; ++i)
                        {
                            LOFATAL("UFATAL--------------------------------------{}", i);
                            lolog::fatal("UFATAL-----------------------------------");
                        }
                    });

    th1.join();
    th2.join();
    th3.join();
    th4.join();
}
