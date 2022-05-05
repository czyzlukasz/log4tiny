#include "log4tiny.hpp"

int main() {
//  tinylog("dudassdadsa format %, test: %", 420, 342)
//  tinylog("testing format %", 32)
//
//  tinylog("testing format %u", 4)
//  tinylog("%+8u ghf fgh fgh", 8, 4, 4, 4, 4)


  const std::string_view format = "%+888.34LF ghf fgh fgh";
//  if(const auto post_start_substring = consume_start_character(format)) {
//    const auto post_flags_substring = consume_flags_if_any(*post_start_substring);
//    const auto [post_width_substring, width_argument] = consume_width_if_any(*post_flags_substring);
//    const auto [post_precision_substring, precision_argument] = consume_precision_if_any(*post_flags_substring);
//    return {true, width_argument + precision_argument};

  const auto post_start_substring = consume_start_character(format);
  std::cout << post_start_substring.value() << std::endl;
  const auto post_flags_substring = consume_flags_if_any(post_start_substring.value());
  std::cout << post_flags_substring.value() << std::endl;
  const auto [post_width_substring, width_argument] = consume_width_if_any(post_flags_substring.value());
  std::cout << post_width_substring.value() << " width_argument: " << width_argument << std::endl;
  const auto [post_precision_substring, precision_argument] = consume_precision_if_any(post_width_substring.value());
  std::cout << post_precision_substring.value() << " width_argument: " << precision_argument << std::endl;
  const auto [post_length_substring, specifiers] = consume_length_if_any(post_precision_substring.value());
  std::cout << post_length_substring.value() << " specifiers: " << specifiers << std::endl;
  const auto post_specifier_substring = consume_specifier(post_length_substring.value(), specifiers);
  std::cout << post_specifier_substring.value() << std::endl;
  return 0;
}
