#pragma once

#include <iostream>
#include <crc32.hpp>
#include <format_parser.hpp>
#include <data_stream.hpp>
#include <value_encoder.hpp>

namespace log4tiny {

template<const std::string_view &format, typename... T>
constexpr void log(DataStream<std::byte> auto &data_stream, const uint32_t file_hash, const size_t line, const T &... args) {
  ::log4tiny::verify_format_with_arguments<format>(args...);

  data_stream.start_log_entry();
  ValueEncoder<std::byte>{}.encode(data_stream, file_hash);
  ValueEncoder<std::byte>{}.encode(data_stream, line);
  (ValueEncoder<std::byte>{}.encode(data_stream, args), ...);
  data_stream.end_log_entry();
}

#define _TINYLOG_CALCULATE_CRC32(file_path) std::integral_constant<uint32_t, compute_crc32(file_path, sizeof(file_path)-1)>::value

#define tinylog(...) _TINYLOG_EXTRACT_FORMAT(__VA_ARGS__)

#define _TINYLOG_EXTRACT_FORMAT(sink, format_char_array, ...)                                  \
{                                                                                              \
static constexpr std::string_view format_view = format_char_array;                             \
::log4tiny::log<format_view>(sink, _TINYLOG_CALCULATE_CRC32(__FILE__), __LINE__, __VA_ARGS__); \
}

}
