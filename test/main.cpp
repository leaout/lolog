#include <gtest/gtest.h>
#include <string>
#include "../include/Lolog-Inl.h"

using namespace std;
int main(int argc, char* argv[]) {
    lolog::init_logging("test.log", 5, 200 << 20);
    lolog::set_log_level("Debug");
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
