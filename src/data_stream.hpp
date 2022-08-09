#pragma once

#include <span>

namespace log4tiny {

template <typename BitType>
struct DataStream {
  virtual ~DataStream() = default;
  virtual void add_data_to_stream(std::span<const BitType> data) = 0;
};

}
