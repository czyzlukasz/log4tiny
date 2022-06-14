#pragma once

#include <variant>
#include <concepts>
#include <string>
#include <vector>
#include <algorithm>
#include <complex>
#include <placeholder_attributes.hpp>


namespace log4tiny::matcher {

// Matchers are designed to verify that given type matches specified criteria (i.e. if type is an unsigned integral or a pointer).
// This is done by template specialization that returns true only for types that fit given matcher.
struct SignedIntType : PlaceholderFlag, PlaceholderWidth, PlaceholderPrecision{
  template<typename T>
  constexpr bool matches(T &&t) const {
    return false;
  }

  constexpr bool matches(std::signed_integral auto &&t) const {
    return true;
  }
};

struct UnsignedIntType {
  template<typename T>
  constexpr bool matches(T &&t) const {
    return false;
  }

  constexpr bool matches(std::unsigned_integral auto &&t) const {
    return true;
  }

  PlaceholderFlag flag;
  PlaceholderWidth width;
  PlaceholderPrecision precision;
};

struct FloatingType {
  template<typename T>
  constexpr bool matches(T &&t) const {
    return false;
  }

  constexpr bool matches(std::floating_point auto &&t) const {
    return true;
  }

  PlaceholderFlag flag;
  PlaceholderWidth width;
  PlaceholderPrecision precision;
};

struct CharType {
  template<typename T>
  constexpr bool matches(T &&t) const {
    return false;
  }

  constexpr bool matches(char &&t) const {
    return true;
  }

  constexpr bool matches(unsigned char &&t) const {
    return true;
  }

  PlaceholderFlag flag;
  PlaceholderWidth width;
  PlaceholderPrecision precision;
};

// This matcher provides an extension to C printf - it accepts C const char* as well as C++ std::string
struct StringType {
  template<typename T>
  constexpr bool matches(T &&t) const {
    return false;
  }

  constexpr bool matches(std::string &&t) const {
    return true;
  }

  constexpr bool matches(const char *&&t) const {
    return true;
  }

  PlaceholderFlag flag;
  PlaceholderWidth width;
  PlaceholderPrecision precision;
};

struct PointerType {
  template<typename T>
  constexpr bool matches(T &&t) const {
    return false;
  }

  template<typename T>
  constexpr bool matches(T *&&t) const {
    return true;
  }

  PlaceholderFlag flag;
  PlaceholderWidth width;
};

template<typename... AllowedTypes>
struct TypeMatcher {
  constexpr TypeMatcher() : allowed_type_matchers() {};

  template<typename... T>
  constexpr explicit TypeMatcher(T &&... t) : allowed_type_matchers{(std::variant<AllowedTypes...>(t), ...)} {}

  template<typename T>
  constexpr bool matches() const {
    return std::visit([](const auto &type_matcher) -> bool {
      return type_matcher.matches(T{});
    }, allowed_type_matchers);
  }

  template<typename T>
  constexpr const T& get() const {
    return std::get<T>(allowed_type_matchers);
  }

  template<typename Closure>
  void visit_allowed_types(Closure&& closure) {
    std::visit(closure, allowed_type_matchers);
  }

private:
  std::variant<AllowedTypes...> allowed_type_matchers;
};

using PlaceholderType = TypeMatcher<SignedIntType, UnsignedIntType, FloatingType, CharType, StringType, PointerType>;

PlaceholderType placeholder_with_filled_attributes(PlaceholderType&& input, const char flag, const size_t width, const size_t precision) {
  const auto flag_to_option = [](const char flag) -> PlaceholderFlag::Option {
    return {};
  };

  const auto fill_visitor = [flag_to_option, flag, width, precision](auto&& allowed_type){
    using T = std::decay_t<decltype(allowed_type)>;
    if constexpr(std::is_base_of_v<PlaceholderFlag, T>) {
      static_cast<PlaceholderFlag&>(allowed_type).option = flag_to_option(flag);
    }
    if constexpr(std::is_base_of_v<PlaceholderWidth, T>) {
      static_cast<PlaceholderWidth&>(allowed_type).width = width;
    }
    if constexpr(std::is_base_of_v<PlaceholderPrecision, T>) {
      static_cast<PlaceholderPrecision&>(allowed_type).precision = precision;
    }
  };

  input.visit_allowed_types(fill_visitor);
  return input;
}

}
