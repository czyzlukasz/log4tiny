#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <value_encoder.hpp>
#include <utils/span_eq_matcher.hpp>

using namespace log4tiny;
using testing::StrictMock;
using testing::AnyOf;


template<typename ByteType>
struct DataStreamMock : DataStream<ByteType> {
  MOCK_METHOD(void, add_data_to_stream, (std::span<const ByteType> data), (override));
};

struct ValueEncoderTest : testing::Test {
  using ByteType = std::byte;

  void SetUp() override {
    data_stream_mock = std::make_unique<StrictMock<DataStreamMock<ByteType>>>();
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

  EXPECT_CALL(*data_stream_mock, add_data_to_stream(AnyOf(
          SpanEq<std::byte>(as_encoded_bytes(uint8_t(ValueEncoder<uint8_t>::ValueType::INT8), uint8_t(value_to_encode))),
          SpanEq<std::byte>(as_encoded_bytes(uint8_t(ValueEncoder<uint8_t>::ValueType::UINT8), uint8_t(value_to_encode))))));

  EXPECT_TRUE(ValueEncoder<std::byte>{}.encode(*data_stream_mock, value_to_encode));
}

TEST_F(ValueEncoderTest, EncodeNegativeI8) {
  constexpr int64_t value_to_encode = -15;

  EXPECT_CALL(*data_stream_mock,
              add_data_to_stream(SpanEq<std::byte>(as_encoded_bytes(uint8_t(ValueEncoder<uint8_t>::ValueType::INT8), int8_t(value_to_encode)))));

  EXPECT_TRUE(ValueEncoder<std::byte>{}.encode(*data_stream_mock, value_to_encode));
}

TEST_F(ValueEncoderTest, EncodePositiveI8) {
  constexpr int64_t value_to_encode = 45;

  EXPECT_CALL(*data_stream_mock, add_data_to_stream(AnyOf(
          SpanEq<std::byte>(as_encoded_bytes(uint8_t(ValueEncoder<uint8_t>::ValueType::INT8), uint8_t(value_to_encode))),
          SpanEq<std::byte>(as_encoded_bytes(uint8_t(ValueEncoder<uint8_t>::ValueType::UINT8), uint8_t(value_to_encode))))));

  EXPECT_TRUE(ValueEncoder<std::byte>{}.encode(*data_stream_mock, value_to_encode));
}

TEST_F(ValueEncoderTest, EncodeNegativeI32) {
  constexpr int64_t value_to_encode = -2e6;

  EXPECT_CALL(*data_stream_mock,
              add_data_to_stream(SpanEq<std::byte>(as_encoded_bytes(uint8_t(ValueEncoder<uint8_t>::ValueType::INT32), int32_t(value_to_encode)))));

  EXPECT_TRUE(ValueEncoder<std::byte>{}.encode(*data_stream_mock, value_to_encode));
}

TEST_F(ValueEncoderTest, EncodeMinI64) {
  constexpr int64_t value_to_encode = std::numeric_limits<int64_t>::min();

  EXPECT_CALL(*data_stream_mock,
              add_data_to_stream(SpanEq<std::byte>(as_encoded_bytes(uint8_t(ValueEncoder<uint8_t>::ValueType::INT64), int64_t(value_to_encode)))));

  EXPECT_TRUE(ValueEncoder<std::byte>{}.encode(*data_stream_mock, value_to_encode));
}

TEST_F(ValueEncoderTest, EncodeMaxU64) {
  constexpr uint64_t value_to_encode = std::numeric_limits<uint64_t>::max();

  EXPECT_CALL(*data_stream_mock,
              add_data_to_stream(SpanEq<std::byte>(as_encoded_bytes(uint8_t(ValueEncoder<uint8_t>::ValueType::UINT64), int64_t(value_to_encode)))));

  EXPECT_TRUE(ValueEncoder<std::byte>{}.encode(*data_stream_mock, value_to_encode));
}

TEST_F(ValueEncoderTest, EncodeFloat) {
  constexpr float value_to_encode = 1.23456f;

  EXPECT_CALL(*data_stream_mock,
              add_data_to_stream(SpanEq<std::byte>(as_encoded_bytes(uint8_t(ValueEncoder<uint8_t>::ValueType::FLOAT), value_to_encode))));

  EXPECT_TRUE(ValueEncoder<std::byte>{}.encode(*data_stream_mock, value_to_encode));
}

TEST_F(ValueEncoderTest, EncodeDouble) {
  constexpr double value_to_encode = 1.2345678910;

  EXPECT_CALL(*data_stream_mock,
              add_data_to_stream(SpanEq<std::byte>(as_encoded_bytes(uint8_t(ValueEncoder<uint8_t>::ValueType::DOUBLE), value_to_encode))));

  EXPECT_TRUE(ValueEncoder<std::byte>{}.encode(*data_stream_mock, value_to_encode));
}

TEST_F(ValueEncoderTest, EncodeBool) {
  constexpr bool value_to_encode = true;

  EXPECT_CALL(*data_stream_mock, add_data_to_stream(AnyOf(
          SpanEq<std::byte>(as_encoded_bytes(uint8_t(ValueEncoder<uint8_t>::ValueType::INT8), uint8_t(value_to_encode))),
          SpanEq<std::byte>(as_encoded_bytes(uint8_t(ValueEncoder<uint8_t>::ValueType::UINT8), uint8_t(value_to_encode))))));

  EXPECT_TRUE(ValueEncoder<std::byte>{}.encode(*data_stream_mock, value_to_encode));
}

TEST_F(ValueEncoderTest, EncodeStdString) {
  const std::string value_to_encode = "This is a test string";
  const auto string_to_vector = [](const std::string &input) -> std::vector<std::byte> {
    std::vector<std::byte> result;
    for (const auto &character: input) {
      result.push_back(static_cast<std::byte>(character));
    }
    return result;
  };

  // Expect a call storing information about value type and length of the string
  EXPECT_CALL(*data_stream_mock, add_data_to_stream(
          SpanEq<std::byte>(as_encoded_bytes(uint8_t(ValueEncoder<uint8_t>::ValueType::STRING), uint32_t(value_to_encode.length())))));
  // Then expect a call with string data
  EXPECT_CALL(*data_stream_mock, add_data_to_stream(SpanEq<std::byte>(string_to_vector(value_to_encode)))).RetiresOnSaturation();

  EXPECT_TRUE(ValueEncoder<std::byte>{}.encode(*data_stream_mock, value_to_encode));
}

TEST_F(ValueEncoderTest, EncodeCString) {
  const char *value_to_encode = "This is a test string";
  const auto string_to_vector = [](const char *input) -> std::vector<std::byte> {
    std::vector<std::byte> result;
    for (size_t it = 0; it < std::strlen(input); ++it) {
      result.push_back(static_cast<std::byte>(input[it]));
    }
    return result;
  };

  // Check ValueEncoderTest_EncodeStdString for explanation
  EXPECT_CALL(*data_stream_mock, add_data_to_stream(
          SpanEq<std::byte>(as_encoded_bytes(uint8_t(ValueEncoder<uint8_t>::ValueType::STRING), uint32_t(std::strlen(value_to_encode))))));
  EXPECT_CALL(*data_stream_mock, add_data_to_stream(SpanEq<std::byte>(string_to_vector(value_to_encode)))).RetiresOnSaturation();

  EXPECT_TRUE(ValueEncoder<std::byte>{}.encode(*data_stream_mock, value_to_encode));
}
