cmake_minimum_required(VERSION 3.15)
project(log4tiny CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(log4tiny INTERFACE src/type_matcher.hpp)
target_include_directories(log4tiny INTERFACE src)

# Sinks
add_library(default_sink src/sink/ostream_sink.cpp)
target_link_libraries(default_sink PRIVATE log4tiny)

add_executable(log4tiny_example_1 examples/example_1.cpp)
target_link_libraries(log4tiny_example_1 log4tiny)

find_library(GTest GTest)

add_executable(tests tests/format_checker_test.cpp tests/value_encoder_test.cpp tests/value_decoder_test.cpp tests/ostream_sink_test.cpp)
target_link_libraries(tests gtest_main gtest gmock log4tiny default_sink)
target_include_directories(tests PRIVATE tests)