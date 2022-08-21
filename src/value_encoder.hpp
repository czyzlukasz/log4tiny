#pragma once

#include <data_stream.hpp>
#include <type_matcher.hpp>
#include <cstring>

namespace log4tiny {

template<typename T>
concept String = std::is_convertible_v<T, std::string_view>;

template<typename ByteType>
struct ValueEncoder {

  enum class ValueType : uint8_t {
    UINT8,
    UINT16,
    UINT32,
    UINT64,
    INT8,
    INT16,
    INT32,
    INT64,
    FLOAT,
    DOUBLE,
    STRING
  };

  bool encode(DataStream<ByteType> auto &data_stream, std::integral auto &value) {
    constexpr auto type_catalog = generate_sorted_type_catalog();

    const auto fits_within_the_range_of_type = [&value](const TypeInformation &type_information) -> bool {
      if constexpr(std::is_unsigned_v<decltype(value)>) {
        return value <= type_information.max_possible_value;
      } else {
        return value <= static_cast<int64_t>(type_information.max_possible_value) and value >= type_information.min_possible_value;
      }
    };

    const auto smallest_possible_type = std::ranges::find_if(type_catalog, fits_within_the_range_of_type);
    if (smallest_possible_type == type_catalog.cend()) {
      // Could not find any supported type that would fit specified value
      return false;
    }

    add_type_information_to_buffer(smallest_possible_type->value_type);
    add_value_to_buffer(value, smallest_possible_type->size_of_encoded_value);

    const auto encoded_data_size_with_type_info = smallest_possible_type->size_of_encoded_value + sizeof(ValueType);
    const auto result = std::span(encoded_data_buffer.cbegin(), encoded_data_buffer.cbegin() + encoded_data_size_with_type_info);
    data_stream.add_data_to_stream(result);
    return true;
  }

  bool encode(DataStream<ByteType> auto &data_stream, std::floating_point auto &value) {
    add_type_information_to_buffer(std::is_same_v<std::decay_t<decltype(value)>, float> ? ValueType::FLOAT : ValueType::DOUBLE);
    add_value_to_buffer(value, sizeof(value));

    const auto encoded_data_size_with_type_info = sizeof(value) + sizeof(ValueType);
    const auto result = std::span(encoded_data_buffer.cbegin(), encoded_data_buffer.cbegin() + encoded_data_size_with_type_info);
    data_stream.add_data_to_stream(result);
    return true;
  }

  bool encode(DataStream<ByteType> auto &data_stream, const String auto &value) {
    const std::string_view sv = value;
    const uint32_t string_length = sv.length();

    // Store information about type and the length of the string
    add_type_information_to_buffer(ValueType::STRING);
    add_value_to_buffer(string_length, sizeof(string_length));

    const auto encoded_length_with_type_info = sizeof(ValueType) + sizeof(string_length);
    const auto result = std::span(encoded_data_buffer.cbegin(), encoded_data_buffer.cbegin() + encoded_length_with_type_info);
    data_stream.add_data_to_stream(result);

    // TODO: thats uglyyy
    const auto string_begin = reinterpret_cast<const std::byte *>(sv.data());
    const auto string_span = std::span(string_begin, string_begin + string_length);
    data_stream.add_data_to_stream(string_span);
    return true;
  }

private:
  struct TypeInformation {
    uint64_t max_possible_value;
    int64_t min_possible_value;
    ValueType value_type;
    size_t size_of_encoded_value;
  };

  template<typename T>
  static consteval ValueType type_to_value_type() {
    if constexpr(std::is_same_v<T, int8_t>) return ValueType::INT8;
    else if constexpr(std::is_same_v<T, int16_t>) return ValueType::INT16;
    else if constexpr(std::is_same_v<T, int32_t>) return ValueType::INT32;
    else if constexpr(std::is_same_v<T, int64_t>) return ValueType::INT64;
    else if constexpr(std::is_same_v<T, uint8_t>) return ValueType::UINT8;
    else if constexpr(std::is_same_v<T, uint16_t>) return ValueType::UINT16;
    else if constexpr(std::is_same_v<T, uint32_t>) return ValueType::UINT32;
    else if constexpr(std::is_same_v<T, uint64_t>) return ValueType::UINT64;
    else if constexpr(std::is_same_v<T, float>) return ValueType::FLOAT;
    else if constexpr(std::is_same_v<T, double>) return ValueType::DOUBLE;
    else {
      []<bool flag = false> { static_assert(flag, "Specified type is not supported"); }();
    }
  };

  template<typename T>
  static constexpr TypeInformation generate_type_information_for() {
    return TypeInformation{
            .max_possible_value = std::numeric_limits<T>::max(),
            .min_possible_value = std::numeric_limits<T>::min(),
            .value_type = type_to_value_type<T>(),
            .size_of_encoded_value = sizeof(T) * sizeof(ByteType)
    };
  }

  static constexpr auto generate_sorted_type_catalog() {
    std::array type_size_catalog = {
            generate_type_information_for<int8_t>(),
            generate_type_information_for<int16_t>(),
            generate_type_information_for<int32_t>(),
            generate_type_information_for<int64_t>(),
            generate_type_information_for<uint8_t>(),
            generate_type_information_for<uint16_t>(),
            generate_type_information_for<uint32_t>(),
            generate_type_information_for<uint64_t>()
    };
    std::ranges::sort(type_size_catalog, {}, &TypeInformation::size_of_encoded_value);

    return type_size_catalog;
  }

  static constexpr auto get_largest_size_of_encoded_value() {
    return generate_sorted_type_catalog().rbegin()->size_of_encoded_value;
  }

  void add_type_information_to_buffer(const ValueType &value_type) {
    encoded_data_buffer.at(0) = static_cast<ByteType>(value_type);
  }

  template<typename T>
  void add_value_to_buffer(const T &value, const size_t &size_of_encoded_value) {
    T helper_array[] = {value};
    const auto value_span = std::as_bytes(std::span(helper_array));
    std::copy(value_span.begin(), value_span.end(), encoded_data_buffer.begin() + 1);
  }

  // Buffer for data before inserting it into data_stream by encode method. Note that we assume worst case scenario (the largest value that we can encode)
  // plus additional info for storing the type that we encode. For more info see encode method.
  static inline std::array<ByteType, sizeof(ValueType) + get_largest_size_of_encoded_value()> encoded_data_buffer;
};

}
