#pragma once

#include <iostream>
#include <crc32.hpp>
#include <format_parser.hpp>

namespace log4tiny {

template<const std::string_view &format, typename... T>
consteval void log(const uint32_t file_hash, const size_t line, const T &... args) {
  ::log4tiny::verify_format_with_arguments<format>(args...);
}

#define tinylog(...) _TINYLOG_EXTRACT_FORMAT(__VA_ARGS__)

#define _TINYLOG_EXTRACT_FORMAT(format_char_array, ...)                                  \
{                                                                                        \
static constexpr std::string_view format_view = format_char_array;                       \
::log4tiny::log<format_view>(compute_crc32(__FILE__, sizeof(__FILE__) - 1), __LINE__, __VA_ARGS__); \
}

}
