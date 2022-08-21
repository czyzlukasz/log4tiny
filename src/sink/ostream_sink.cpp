#include <sink/ostream_sink.hpp>
#include <iostream>
#include <ranges>
#include <vector>
#include <algorithm>

namespace log4tiny {

OstreamSink::OstreamSink(std::ostream &destination_stream_) : destination_stream(destination_stream_) {

}

void OstreamSink::start_log_entry() {

}

void OstreamSink::add_data_to_stream(const std::span<const std::byte> data) {
  for (const auto &byte: data) {
    destination_stream << static_cast<std::ostream::char_type>(byte);
  }
}

void OstreamSink::end_log_entry() {
  destination_stream.flush();
}

}
