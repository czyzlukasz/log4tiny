#pragma once

#include <variant>
#include <concepts>
#include <string>
#include <vector>
#include <algorithm>

namespace log4tiny::matcher {

// Matchers are designed to verify that given type matches specified criteria (i.e. if type is an unsigned integral or a pointer).
// This is done by template specialization that returns true only for types that fit given matcher.
struct SignedIntType {
  template<typename T>
  static constexpr bool matches(T &&t) {
    return false;
  }

  static constexpr bool matches(std::signed_integral auto &&t) {
    return true;
  }
};

struct UnsignedIntType {
  template<typename T>
  static constexpr bool matches(T &&t) {
    return false;
  }

  static constexpr bool matches(std::unsigned_integral auto &&t) {
    return true;
  }
};

struct FloatingType {
  template<typename T>
  static constexpr bool matches(T &&t) {
    return false;
  }

  static constexpr bool matches(std::floating_point auto &&t) {
    return true;
  }
};

struct CharType {
  template<typename T>
  static constexpr bool matches(T &&t) {
    return false;
  }

  static constexpr bool matches(char &&t) {
    return true;
  }

  static constexpr bool matches(unsigned char &&t) {
    return true;
  }
};

// This matcher provides an extension to C printf - it accepts C const char* as well as C++ std::string
struct StringType {
  template<typename T>
  static constexpr bool matches(T &&t) {
    return false;
  }

  static constexpr bool matches(std::string &&t) {
    return true;
  }

  static constexpr bool matches(const char *&&t) {
    return true;
  }
};

struct PointerType {
  template<typename T>
  static constexpr bool matches(T &&t) {
    return false;
  }

  template<typename T>
  static constexpr bool matches(T *&&t) {
    return true;
  }
};

template<typename... AllowedTypes>
struct TypeMatcher {
  constexpr TypeMatcher() : allowed_type_matchers() {};

  template<typename... T>
  constexpr explicit TypeMatcher(T &&... t) : allowed_type_matchers{(std::variant<AllowedTypes...>(t), ...)} {}

  template<typename T>
  constexpr bool matches() const {
    return std::ranges::any_of(allowed_type_matchers, [](const auto &type_matcher_variant) -> bool {
      return std::visit([](const auto &type_matcher) -> bool {
        return type_matcher.matches(T{});
      }, type_matcher_variant);
    });
  }

private:
  std::vector<std::variant<AllowedTypes...>> allowed_type_matchers;
};

using PlaceholderType = TypeMatcher<SignedIntType, UnsignedIntType, FloatingType, CharType, StringType, PointerType>;

}
