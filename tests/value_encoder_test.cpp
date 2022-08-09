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
  void SetUp() override {
    data_stream_mock = std::make_unique<StrictMock<DataStreamMock<uint8_t>>>();
  }

  template<typename... T>
  std::vector<uint8_t> as_encoded_bytes(T &&... values) {
    std::vector<uint8_t> result;
    const auto convert_to_bytes = [&result](auto &&value) {
      const auto bytes = std::bit_cast<std::array<uint8_t, sizeof(value)>>(value);
      std::ranges::copy(bytes, std::back_inserter(result));
    };

    (convert_to_bytes(values), ...);
    return result;
  }

protected:
  std::unique_ptr<StrictMock<DataStreamMock<uint8_t>>> data_stream_mock;
};

TEST_F(ValueEncoderTest, EncodeSigned8) {
  constexpr int8_t magic_number = -12;

  EXPECT_CALL(*data_stream_mock, add_data_to_stream(SpanEq<uint8_t>({11, 24, 3}))).Times(1);
  ValueEncoder::encode<matcher::SignedIntType>(magic_number, *data_stream_mock);
}