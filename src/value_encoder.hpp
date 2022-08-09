#pragma once

#include <data_stream.hpp>
#include <type_matcher.hpp>

namespace log4tiny {

struct ValueEncoder {
  template<typename Matcher, typename T, typename ByteType>
  static void encode(T &&value, DataStream<ByteType> &data_stream) {
    if constexpr(std::is_same_v<Matcher, matcher::SignedIntType>) {
      encode_signed(value, data_stream);
    }
  }

private:
  template<typename T, typename ByteType>
  static void encode_signed(T &&value, DataStream<ByteType> &data_stream) {
    enum class SignedType : ByteType {
      INT8 = 0,
      INT16 = 1,
      INT32 = 2,
      INT64 = 3
    };

    const auto type_to_enum = [&value]() -> SignedType {
      if (value <= std::numeric_limits<int8_t>::max() and value >= std::numeric_limits<int8_t>::min()) {
        return SignedType::INT8;
      } else if (value <= std::numeric_limits<int16_t>::max() and value >= std::numeric_limits<int16_t>::min()) {
        return SignedType::INT16;
      } else if (value <= std::numeric_limits<int32_t>::max() and value >= std::numeric_limits<int32_t>::min()) {
        return SignedType::INT32;
      } else if (value <= std::numeric_limits<int64_t>::max() and value >= std::numeric_limits<int64_t>::min()) {
        return SignedType::INT64;
      }
      // TODO: add custom position for encoding integers greater than INT64
      return SignedType::INT64;
    };

    const auto get_size_of_data = [](const SignedType &type) -> size_t {
      switch (type) {
        case SignedType::INT8:
          return 1;
        case SignedType::INT16:
          return 2;
        case SignedType::INT32:
          return 4;
        case SignedType::INT64:
          return 8;
        default:
          return 0;
      }
    };

    const auto data_type = type_to_enum();
    const auto type_used_for_encoding = std::bit_cast<ByteType>(data_type);
    const auto encoded_type_used = std::span(&type_used_for_encoding, sizeof(ByteType));
    data_stream.add_data_to_stream(encoded_type_used);

    const auto data_bits = std::bit_cast<ByteType>(value);
    const auto data = std::span(&data_bits, get_size_of_data(data_type));
    data_stream.add_data_to_stream(data);
  }
};

}
