#pragma once

#include <string_view>
#include <algorithm>
#include <optional>
#include <ranges>

namespace log4tiny {

// Functions named "consume_" work by checking provided substring from the beginning and returning:
// 1. Another substring with or without first character(s) in case of functions with "_if_any" suffix
// 2. Another substring in case of successful character consumption or std::nullopt otherwise
// Note that "consumer" functions do not check string for bound conditions - this is done for readability and clarity
// reasons. In case of out-of-bound read exception is thrown that can be caught and handled accordingly

constexpr std::optional<std::string_view> consume_character(const std::string_view &format, const char character) {
  if (*format.cbegin() == character) {
    return format.substr(1);
  }
  return std::nullopt;
}

// Consume character that fits in range first_character <= x <= last_character
constexpr std::optional<std::string_view>
consume_character_from_range(const std::string_view &format, const char first_character,
                             const char last_character) {
  if (*format.cbegin() >= first_character and *format.cbegin() <= last_character) {
    return format.substr(1);
  }
  return std::nullopt;
}

// Consume character if matches any character provided in parameter pack
template<typename... T>
requires(std::is_same_v<T, char> and...)
constexpr std::optional<std::string_view>
consume_character_from_set(const std::string_view &format, T... characters) {
  if (((*format.cbegin() == characters) or ...)) {
    return format.substr(1);
  }
  return std::nullopt;
}

// Consume character if matches any character provided in container
template<typename Container>
constexpr std::optional<std::string_view>
consume_character_from_set(const std::string_view &format, const Container &characters) {
  if (std::ranges::find(characters, *format.cbegin()) != characters.end()) {
    return format.substr(1);
  }
  return std::nullopt;
}

// Consume entire string if matches exactly
constexpr std::optional<std::string_view>
consume_string(const std::string_view &format, const std::string_view &string_to_consume) {
  if (format.find(string_to_consume) == 0) {
    return format.substr(string_to_consume.length());
  }
  return std::nullopt;
}

template<typename T, typename... Args>
concept IsCharacterConsumer = requires(T t, const std::string_view &format, const Args &... args) {
  { t(format, args...) } -> std::same_as<std::optional<std::string_view>>;
};

// Recursive caller for any character consumer. This function will call function recursively until std::nullopt
// is achieved
template<typename Function, typename... Args>
requires IsCharacterConsumer<Function, Args...>

constexpr std::optional<std::string_view>
consume_repeatedly(Function function, const std::string_view &format, const Args &... args) {
  while (true) {
    const std::optional<std::string_view> return_value = function(format, args...);

    if (not return_value or return_value.value() == format) {
      return format;
    } else {
      return consume_repeatedly(function, return_value.value(), args...);
    }
  }
}

constexpr std::optional<std::string_view> consume_start_character(const std::string_view &format) {
  return consume_character(format, '%');
}

constexpr std::string_view consume_flags_if_any(const std::string_view &format) {
  const auto substring = consume_character_from_set(format, '+', '-', ' ', '#', '0');
  return substring.value_or(format);
}

// Consume width specification and return information about additional argument required (in case of '*')
constexpr auto consume_width_if_any(const std::string_view &format) {
  struct ReturnValue {
    std::string_view substring;
    size_t number_of_additional_arguments;
  };

  if (const auto additional_argument_substring = consume_character(format, '*')) {
    return ReturnValue{.substring = additional_argument_substring.value(), .number_of_additional_arguments = 1};
  } else if (const auto substring = consume_repeatedly(consume_character_from_range, format, '0', '9')) {
    return ReturnValue{.substring = substring.value(), .number_of_additional_arguments = 0};

  }
  return ReturnValue{.substring = format, .number_of_additional_arguments = 0};
}

// Consume precision specification and return information about additional argument required (in case of '*')
constexpr auto consume_precision_if_any(const std::string_view &format) {
  struct ReturnValue {
    std::string_view substring;
    size_t number_of_additional_arguments;
  };
  if (const auto substring_without_precision_specifier = consume_character(format, '.')) {
    if (const auto additional_argument_substring = consume_character(
            substring_without_precision_specifier.value(),
            '*')) {
      return ReturnValue{.substring = additional_argument_substring.value(), .number_of_additional_arguments = 1};
    } else if (const auto substring = consume_repeatedly(consume_character_from_range,
                                                         substring_without_precision_specifier.value(), '0',
                                                         '9')) {
      return ReturnValue{.substring = substring.value(), .number_of_additional_arguments = 0};
    }
  }
  return ReturnValue{.substring = format, .number_of_additional_arguments = 0};
}

enum class Specifier : char {
  d = 'd',
  i = 'i',
  u = 'u',
  o = 'o',
  x = 'x',
  X = 'X',
  f = 'f',
  F = 'F',
  e = 'e',
  E = 'E',
  g = 'g',
  G = 'G',
  a = 'a',
  A = 'A',
  c = 'c',
  s = 's',
  p = 'p',
  n = 'n'
};

// Consume length specifier and return information about allowed specifiers that are expected
constexpr auto consume_length_if_any(const std::string_view &format) {
  struct ReturnValue {
    std::string_view substring;
    std::vector<Specifier> allowed_specifiers;
  };

  if (const auto substring = consume_string(format, "hh")) {
    return ReturnValue{.substring = substring.value(), .allowed_specifiers = {Specifier::d, Specifier::i, Specifier::u, Specifier::o, Specifier::x,
                                                                              Specifier::X, Specifier::n}};
  }
  if (const auto substring = consume_string(format, "ll")) {
    return ReturnValue{.substring = substring.value(), .allowed_specifiers = {Specifier::d, Specifier::i, Specifier::u, Specifier::o, Specifier::x,
                                                                              Specifier::X, Specifier::n}};
  }
  if (const auto substring = consume_character(format, 'l')) {
    return ReturnValue{.substring = substring.value(), .allowed_specifiers = {Specifier::d, Specifier::i, Specifier::u, Specifier::o, Specifier::x,
                                                                              Specifier::X, Specifier::c, Specifier::s, Specifier::n}};
  }
  if (const auto substring = consume_character(format, 'L')) {
    return ReturnValue{.substring = substring.value(), .allowed_specifiers = {Specifier::f, Specifier::F, Specifier::e, Specifier::E, Specifier::g,
                                                                              Specifier::G, Specifier::a, Specifier::A}};
  }
  if (const auto substring = consume_character_from_set(format, 'h', 'j', 'z', 't')) {
    return ReturnValue{.substring = substring.value(), .allowed_specifiers = {Specifier::d, Specifier::i, Specifier::u, Specifier::o, Specifier::x,
                                                                              Specifier::X, Specifier::n}};
  }
  return ReturnValue{.substring = format, .allowed_specifiers = {Specifier::d, Specifier::i, Specifier::u, Specifier::o, Specifier::x, Specifier::X,
                                                                 Specifier::f, Specifier::F, Specifier::e, Specifier::E, Specifier::g, Specifier::G,
                                                                 Specifier::a, Specifier::A, Specifier::c, Specifier::s, Specifier::p, Specifier::n}};
}

constexpr std::vector<char> specifiers_to_characters(const std::vector<Specifier> &specifiers) {
  std::vector<char> result{};
  std::ranges::transform(specifiers, std::back_inserter(result), [](const Specifier &specifier) { return static_cast<char>(specifier); });
  return result;
}

struct PlaceholderTypeMatcher {
  template <typename T>
  bool match(T t) {
    return false;
  }

  bool match(int i) {
    return true;
  }

};

constexpr std::optional<std::string_view>
consume_specifier(const std::string_view &format, const std::vector<Specifier> &allowed_specifiers) {
  const auto specifier_characters = specifiers_to_characters(allowed_specifiers);
  if (consume_character_from_set(format, specifier_characters)) {
    return format.substr(1);
  }
  return std::nullopt;
}

// Try to match %[flags][width][.precision][length]specifier prototype and return information about additional arguments
// required (if needed) as well as length of parsed placeholder
constexpr auto parse_first_placeholder(const std::string_view &format) {
  struct ReturnValue {
    bool is_valid;
    std::vector<PlaceholderTypeMatcher> type_matchers;
    long placeholder_length;
  };

  try {
    if (const auto post_start_substring = consume_start_character(format)) {
      const auto post_flags_substring = consume_flags_if_any(post_start_substring.value());
      const auto [post_width_substring, width_argument] = consume_width_if_any(post_flags_substring);
      const auto [post_precision_substring, precision_argument] = consume_precision_if_any(
              post_width_substring);
      const auto [post_length_substring, allowed_specifiers] = consume_length_if_any(
              post_precision_substring);
      if (const auto post_specifier_substring = consume_specifier(post_length_substring,
                                                                  allowed_specifiers)) {
        return ReturnValue{.is_valid = true,
                .type_matchers = {PlaceholderTypeMatcher{}},
                .placeholder_length = std::distance(format.cbegin(), post_specifier_substring->cbegin())};
      }
    }
  }
  catch (const std::exception &exception) {
  }
  return ReturnValue{.is_valid = false, .type_matchers = {}, .placeholder_length = 0};
}

constexpr std::string_view skip_escaped_starting_character(const std::string_view &format) {
  if (format.starts_with("%%") or format.starts_with("\%")) {
    return format.substr(1);
  }
  return format;
}

// Return number of valid placeholders in given string
constexpr std::vector<PlaceholderTypeMatcher> parse_format_to_placeholder_matchers(const std::string_view &format) {
  std::vector<PlaceholderTypeMatcher> result{};

  auto substring = skip_escaped_starting_character(format);
  while (not substring.empty()) {
    if (const auto [is_valid, new_type_matcher, length_of_placeholder] = parse_first_placeholder(substring); is_valid) {
      std::ranges::move(new_type_matcher, std::back_inserter(result));
      substring.remove_prefix(length_of_placeholder);
    } else {
      substring.remove_prefix(1);
    }
    substring = skip_escaped_starting_character(substring);
  }

  return result;
}

template<const std::string_view &format, typename... T>
constexpr void verify_format_with_arguments(const T &... args) {
  static_assert(sizeof...(T) == parse_format_to_placeholder_matchers(format).size(),
                "Number of argument passed does not match the number of placeholders in the format");
}

}
