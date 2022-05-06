cmake_minimum_required(VERSION 3.15)
project(log4tiny CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(log4tiny INTERFACE)
target_include_directories(log4tiny INTERFACE src)

add_executable(log4tiny_example_1 examples/example_1.cpp)
target_link_libraries(log4tiny_example_1 log4tiny)

find_library(GTest GTest)

add_executable(tests tests/format_checker_test.cpp)
target_link_libraries(tests gtest_main gtest log4tiny)
