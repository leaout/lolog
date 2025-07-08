cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME lolog)
project(${PROJECT_NAME})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation directory")


set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/lolog")  
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

file(GLOB_RECURSE HEADERS *.h)
source_group("Header Files" FILES ${HEADERS})
aux_source_directory(./src DIR_SRCS)
add_library(${PROJECT_NAME} SHARED ${DIR_SRCS} ${HEADERS})

find_package(Threads)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

include_directories(${CMAKE_SOURCE_DIR})
link_directories(${CMAKE_BINARY_DIR})
aux_source_directory(./bench DIR_BENCH)
add_executable(bench ${DIR_BENCH})
target_link_libraries(bench  ${CMAKE_THREAD_LIBS_INIT})

find_package(GTest)
if(GTest_FOUND)
    add_subdirectory(test)
endif()

#install
set(CMAKE_INSTALL_RPATH "")
set(INSTALL_HEAD_FILES include/Lolog.h)
install(FILES ${INSTALL_HEAD_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
INSTALL(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        )
