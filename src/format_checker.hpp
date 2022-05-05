#pragma once
#include <string_view>
#include <algorithm>
#include <optional>

//%[flags][width][.precision][length]specifier

constexpr std::optional<std::string_view> consume_character(const std::string_view& format, const char character) {
  if(*format.cbegin() == character) {
    return format.substr(1);
  }
  return std::nullopt;
}

constexpr std::optional<std::string_view> consume_character_from_range(const std::string_view& format, const char first_character, const char last_character) {
  if(*format.cbegin() >= first_character and *format.cbegin() <= last_character) {
    return format.substr(1);
  }
  return std::nullopt;
}

template <typename... T> requires (std::is_same_v<T, char> and ...)
constexpr std::optional<std::string_view> consume_character_from_set(const std::string_view& format, T... characters) {
  if(((*format.cbegin() == characters) or ...)) {
    return format.substr(1);
  }
  return std::nullopt;
}

template <typename Container>
constexpr std::optional<std::string_view> consume_character_from_set(const std::string_view& format, const Container& characters) {
  if(std::ranges::find(characters, *format.cbegin()) != characters.end()) {
    return format.substr(1);
  }
  return std::nullopt;
}

constexpr std::optional<std::string_view> consume_string(const std::string_view& format, const std::string_view& string_to_consume) {
  if(format.find(string_to_consume) == 0) {
    return format.substr(string_to_consume.length());
  }
  return std::nullopt;
}

template <typename T, typename... Args>
concept IsCharacterConsumer = requires(T t, const std::string_view& format, const Args&... args) {
  {t(format, args...)} -> std::same_as<std::optional<std::string_view>>;
};

template <typename Function, typename... Args> requires IsCharacterConsumer<Function, Args...>
constexpr std::optional<std::string_view> consume_repeatedly(Function function, const std::string_view& format, const Args&... args) {
  while (true) {
    const std::optional<std::string_view> return_value = function(format, args...);

    if(not return_value or return_value.value() == format) {
      return format;
    }
    else {
      return consume_repeatedly(function, return_value.value(), args...);
    }
  }
}

constexpr std::optional<std::string_view> consume_start_character(const std::string_view& format) {
  return consume_character(format, '%');
}

constexpr std::optional<std::string_view> consume_flags_if_any(const std::string_view& format) {
  const auto substring = consume_character_from_set(format, '+', '-', ' ', '#', '0');
  return substring.value_or(format);
}

constexpr std::tuple<std::optional<std::string_view>, size_t> consume_width_if_any(const std::string_view& format) {
  if(const auto additional_argument_substring = consume_character(format, '*')) {
    return {additional_argument_substring.value(), 1};
  }
  else if(const auto substring = consume_repeatedly(consume_character_from_range, format, '0', '9')) {
    return {substring.value(), 0};
  }
  return {format, 0};
}

constexpr std::tuple<std::optional<std::string_view>, size_t> consume_precision_if_any(const std::string_view& format) {
  if(const auto substring_without_precision_specifier = consume_character(format, '.')) {
    if(const auto additional_argument_substring = consume_character(substring_without_precision_specifier.value(), '*')) {
      return {additional_argument_substring.value(), 1};
    }
    else if(const auto substring = consume_repeatedly(consume_character_from_range, substring_without_precision_specifier.value(), '0', '9')) {
      return {substring.value(), 0};
    }
  }
  return {format, 0};
}

constexpr std::tuple<std::optional<std::string_view>, std::string_view> consume_length_if_any(const std::string_view& format) {
  if(const auto substring = consume_string(format, "hh")) {
    return {substring, "diuoxXn"};
  }
  if(const auto substring = consume_string(format, "ll")) {
    return {substring, "diuoxXn"};
  }
  if(const auto substring = consume_character(format, 'l')) {
    return {substring, "diuoxXcsn"};
  }
  if(const auto substring = consume_character(format, 'L')) {
    return {substring, "fFeEgGaA"};
  }
  if(const auto substring = consume_character_from_set(format, 'h', 'j', 'z', 't')) {
    return {substring, "diuoxXn"};
  }
  return {format, "diuoxXnfFeEgGaAcspn"};
}

constexpr std::optional<std::string_view> consume_specifier(const std::string_view& format, const std::string_view& allowed_specifiers) {
  if(consume_character_from_set(format, allowed_specifiers)) {
    return format.substr(1);
  }
  return std::nullopt;
}

constexpr std::tuple<bool, size_t> is_valid_placeholder(const std::string_view& format) {
  try {
    if(const auto post_start_substring = consume_start_character(format)) {
      const auto post_flags_substring = consume_flags_if_any(post_start_substring.value());
      const auto [post_width_substring, width_argument] = consume_width_if_any(post_flags_substring.value());
      const auto [post_precision_substring, precision_argument] = consume_precision_if_any(post_width_substring.value());
      const auto [post_length_substring, allowed_specifiers] = consume_length_if_any(post_precision_substring.value());
      const auto post_specifier_substring = consume_specifier(post_length_substring.value(), allowed_specifiers);
      return {true, width_argument + precision_argument};
    }
  }
  catch (const std::exception& exception){
  }
  return {true, 0};
}

constexpr size_t count_number_of_valid_placeholders(const std::string_view& format, size_t valid_placeholders = 0, size_t cursor = 1) {
  while (cursor < format.size()) {
    auto substring = format.substr(cursor);
    if(const auto [is_valid, number_of_additional_arguments] = is_valid_placeholder(substring); is_valid) {
      return count_number_of_valid_placeholders(substring, valid_placeholders + 1 + number_of_additional_arguments, cursor + 1);
    }
    count_number_of_valid_placeholders(substring, valid_placeholders, cursor + 1);
  }
  return valid_placeholders;
}

template <const std::string_view& format, typename... T>
constexpr void verify_format_with_arguments(const T&... args) {
  static_assert(sizeof...(T) == count_number_of_valid_placeholders(format),
          "Number of argument passed does not match the number of placeholders in the format");
}
