#pragma once

#include <span>

namespace log4tiny {

template<typename T, typename ByteType>
concept DataStream = requires(T &t, const std::span<const ByteType> &data) {
  { t.start_log_entry() } -> std::same_as<void>;
  { t.add_data_to_stream(data) } -> std::same_as<void>;
  { t.end_log_entry() } -> std::same_as<void>;
};

}
