#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <value_encoder.hpp>
#include <utils/span_eq_matcher.hpp>

using namespace log4tiny;
using testing::StrictMock;


template<typename ByteType>
struct DataStreamMock : DataStream<ByteType> {
  MOCK_METHOD(void, add_data_to_stream, (std::span<const ByteType> data), (override));
};

struct ValueEncoderTest : testing::Test {
  using ByteType = uint8_t;

  void SetUp() override {
    data_stream_mock = std::make_unique<StrictMock<DataStreamMock<uint8_t>>>();
  }

protected:
  template<typename... T>
  std::vector<ByteType> as_encoded_bytes(T &&... values) {
    std::vector<ByteType> result;
    const auto convert_to_bytes = [&result](auto &&value) {
      const auto bytes = std::bit_cast<std::array<ByteType, sizeof(value) * sizeof(ByteType)>>(value);
      std::ranges::copy(bytes, std::back_inserter(result));
    };

    (convert_to_bytes(values), ...);
    return result;
  }

  std::unique_ptr<StrictMock<DataStreamMock<ByteType>>> data_stream_mock;
};

TEST_F(ValueEncoderTest, EncodeZero) {
  constexpr int64_t value_to_encode = 0;

  EXPECT_CALL(*data_stream_mock, add_data_to_stream(SpanEq<uint8_t>(as_encoded_bytes(uint8_t(0), uint8_t(value_to_encode)))));

  ValueEncoder<uint8_t>{}.encode(*data_stream_mock, value_to_encode);
}
