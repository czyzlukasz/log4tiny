#pragma once

#include <iostream>
#include <crc32.hpp>
#include <format_checker.hpp>

namespace log4tiny {

template<const std::string_view &format, typename... T>
constexpr void log(const uint32_t file_hash, const size_t line, const T &... args) {
  ::log4tiny::verify_format_with_arguments<format>(args...);
}

#define _TINYLOG_CALCULATE_CRC32(file_path) std::integral_constant<uint32_t, compute_crc32(file_path, sizeof(file_path)-1)>::value

#define tinylog(...) _TINYLOG_EXTRACT_FORMAT(__VA_ARGS__)

#define _TINYLOG_EXTRACT_FORMAT(format_char_array, ...)                                  \
{                                                                                        \
static constexpr std::string_view format_view = format_char_array;                       \
::log4tiny::log<format_view>(_TINYLOG_CALCULATE_CRC32(__FILE__), __LINE__, __VA_ARGS__); \
}

}
