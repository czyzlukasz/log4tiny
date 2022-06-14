#pragma once

namespace log4tiny::matcher {

constexpr auto max_placeholder_width_or_precision = std::numeric_limits<size_t>::max();

struct PlaceholderWidth {
  PlaceholderWidth(const size_t width_ = max_placeholder_width_or_precision) : width(width_) {}

  operator size_t() const {
    return width;
  }

  size_t width;
};

struct PlaceholderPrecision {
  PlaceholderPrecision(const size_t precision_ = max_placeholder_width_or_precision) : precision(precision_) {}

  operator size_t() const {
    return precision;
  }

  size_t precision;
};

struct PlaceholderFlag {
  enum class Option {
    NONE,
    LEFT_JUSTIFY,
    FORCE_SIGN,
    FORCE_BLANK_SPACE,
    FORCE_HEX_PREFIX,
    PAD_WITH_ZEROS
  };

  PlaceholderFlag(const Option option_ = Option::NONE) : option(option_) {}

  Option option;
};

}
