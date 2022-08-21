#pragma once

#include <data_stream.hpp>
#include <ostream>

namespace log4tiny {

struct OstreamSink {
  explicit OstreamSink(std::ostream &destination_stream);

  void start_log_entry();

  void add_data_to_stream(std::span<const std::byte> data);

  void end_log_entry();

private:
  std::ostream &destination_stream;
};

}
