#pragma once

#include <gmock/gmock.h>
#include <vector>
#include <ostream>
#include <span>
#include <cstddef>

using testing::Matcher;

template<typename T>
class SpanEqMatcher {
public:
  using is_gtest_matcher = void;

  explicit SpanEqMatcher(std::vector<T> &&expected_data_) : expected_data(std::move(expected_data_)) {}

  bool MatchAndExplain(std::span<const T> argument, std::ostream *os) const {
    const bool are_equal = std::ranges::equal(argument, expected_data);
    if (os != nullptr) {

      if (argument.size() != expected_data.size()) {
        *os << "Containers size differ: expected " << expected_data.size() << " items, got " << argument.size() << " instead. Items are:"
            << std::endl;
      } else {
        *os << "Containers differ. Items are:" << std::endl;
      }
      print_container(*os, argument);
    }

    return are_equal;
  }

  void DescribeTo(std::ostream *os) const {
    if (os != nullptr) {
      *os << "Container with data:" << std::endl;
      print_container(*os, expected_data);
    }
  }

  void DescribeNegationTo(std::ostream *os) const {}

private:
  void print_container(std::ostream &os, const auto &container) const {
    for (const auto &item: container) {
      os << "0x" << std::uppercase << std::hex << static_cast<unsigned>(item) << ", ";
    }
  }

  std::vector<T> expected_data;
};

template<typename T>
Matcher<std::span<const T>> SpanEq(std::vector<T> &&expected_data) {
  return SpanEqMatcher(std::move(expected_data));
}
