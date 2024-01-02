cmake_minimum_required(VERSION 3.10)
project(test_lolog)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/test)
link_directories(${CMAKE_SOURCE_DIR}/cmake-build-debug)

find_path(GTEST_HEADER NAMES gtest/gtest.h)
find_library(GTEST_LIB NAMES gtest)
find_library(GTEST_MAIN_LIB NAMES gtest_main)

set(CMAKE_CPP_FLAGS "-DGFLAGS_NS=${GFLAGS_NS}")
set(CMAKE_CPP_FLAGS "${CMAKE_CPP_FLAGS} -D__const__= -D_GNU_SOURCE -DUSE_SYMBOLIZE -DNO_TCMALLOC -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -DUNIT_TEST -g -Dprotected=public -D__STRICT_ANSI__")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CPP_FLAGS} -pipe -Wall -W -fPIC -fstrict-aliasing -Wno-invalid-offsetof -Wno-unused-parameter -fno-omit-frame-pointer -Wno-unused-result")

macro(use_cxx17)
    if(CMAKE_VERSION VERSION_LESS "3.1.3")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    else()
        set(CMAKE_CXX_STANDARD 17)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
    endif()
endmacro(use_cxx17)

use_cxx17()

set(UNIT_TEST_PROJECT unit-test)
file(GLOB TEST_ULTIPA_SRCS "test_*.cpp")
add_executable(${UNIT_TEST_PROJECT} main.cpp ${TEST_ULTIPA_SRCS})

target_link_libraries(${UNIT_TEST_PROJECT} ${GTEST_MAIN_LIB} ${GTEST_LIB} -lpthread -llolog)
