#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sink/ostream_sink.hpp>
#include "log4tiny.hpp"

using namespace log4tiny;

struct DefaultSinkTest : testing::Test {
  const std::span<std::byte> data{};
};

TEST_F(DefaultSinkTest, EmptyLog) {
  OstreamSink err_stream(std::cout);
  tinylog(err_stream, "Dupa %u", 69);
}
