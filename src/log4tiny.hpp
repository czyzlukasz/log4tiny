#pragma once
#include <iostream>
#include <crc32.hpp>
#include <format_checker.hpp>

template <const std::string_view& format, typename... T>
constexpr void log(const uint32_t file_hash, const size_t line, const T&... args) {
//  std::cout << file_hash << ":" << line << " -> " << format << std::endl;
  verify_format_with_arguments<format>(args...);
}

#define CALCULATE_CRC32(file_path)                                                           \
    std::integral_constant<uint32_t, compute_crc32(file_path, sizeof(file_path)-1)>::value

#define tinylog(...) \
_TINYLOG_EXTRACT_FORMAT(__VA_ARGS__)

#define _TINYLOG_EXTRACT_FORMAT(format_char_array, ...) \
{                                            \
static constexpr std::string_view format_view = format_char_array;                                             \
log<format_view>(CALCULATE_CRC32(__FILE__), __LINE__, __VA_ARGS__); \
}
