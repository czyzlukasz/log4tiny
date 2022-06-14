#include <gtest/gtest.h>
#include <format_parser.hpp>

// Verify proper counting of placeholders in format string. Tests were written based on specification from
// https://www.cplusplus.com/reference/cstdio/printf/
// format expects following format: %[flags][width][.precision][length]specifier where only start character (%) and
// specifier is required.

using namespace log4tiny;

struct PlaceholderCounting : testing::Test {
  constexpr size_t count_number_of_valid_placeholders(const std::string_view &format) {
    return parse_format_to_placeholder_matchers(format).size();
  }
};

TEST_F(PlaceholderCounting, NoArgument) {
  EXPECT_EQ(count_number_of_valid_placeholders("Zero arguments"), 0);
  EXPECT_EQ(count_number_of_valid_placeholders(""), 0);
}

TEST_F(PlaceholderCounting, EscapedStartingCharacter) {
  EXPECT_EQ(count_number_of_valid_placeholders("%%"), 0);
  EXPECT_EQ(count_number_of_valid_placeholders("%%u"), 0);
  EXPECT_EQ(count_number_of_valid_placeholders("something %%"), 0);
  EXPECT_EQ(count_number_of_valid_placeholders("something %% something"), 0);
}

TEST_F(PlaceholderCounting, SingleArgument) {
  EXPECT_EQ(count_number_of_valid_placeholders("%u"), 1);
  EXPECT_EQ(count_number_of_valid_placeholders("Simple %u"), 1);
  EXPECT_EQ(count_number_of_valid_placeholders("Simple %u test"), 1);
}

TEST_F(PlaceholderCounting, WithFlag) {
  EXPECT_EQ(count_number_of_valid_placeholders("%-u"), 1);
  EXPECT_EQ(count_number_of_valid_placeholders("%+u"), 1);
  EXPECT_EQ(count_number_of_valid_placeholders("% u"), 1);
  EXPECT_EQ(count_number_of_valid_placeholders("%#u"), 1);
  EXPECT_EQ(count_number_of_valid_placeholders("%0u"), 1);
}

TEST_F(PlaceholderCounting, WithWidth) {
  EXPECT_EQ(count_number_of_valid_placeholders("%6u"), 1);
  EXPECT_EQ(count_number_of_valid_placeholders("%35u"), 1);
  EXPECT_EQ(count_number_of_valid_placeholders("%999999u"), 1);
  // Expect additional argument due to variable width
  EXPECT_EQ(count_number_of_valid_placeholders("%*u"), 2);
}

TEST_F(PlaceholderCounting, WithPrecision) {
  EXPECT_EQ(count_number_of_valid_placeholders("%.4u"), 1);
  EXPECT_EQ(count_number_of_valid_placeholders("%.941u"), 1);
  EXPECT_EQ(count_number_of_valid_placeholders("%.999999u"), 1);
  // Expect additional argument due to variable precision
  EXPECT_EQ(count_number_of_valid_placeholders("%.*u"), 2);
}

TEST_F(PlaceholderCounting, WithLength) {
  // Valid lengths
  EXPECT_EQ(count_number_of_valid_placeholders("%d %i %u %o %x %X %f %F %e %E %g %G %a %A %c %s %p %n"), 18);
  EXPECT_EQ(count_number_of_valid_placeholders("%hhd %hhi %hhu %hho %hhx %hhX %hhn"), 7);
  EXPECT_EQ(count_number_of_valid_placeholders("%hd %hi %hu %ho %hx %hX %hn"), 7);
  EXPECT_EQ(count_number_of_valid_placeholders("%ld %li %lu %lo %lx %lX %lc %ls %ln"), 9);
  EXPECT_EQ(count_number_of_valid_placeholders("%lld %lli %llu %llo %llx %llX %lln"), 7);
  EXPECT_EQ(count_number_of_valid_placeholders("%jd %ji %ju %jo %jx %jX %jn"), 7);
  EXPECT_EQ(count_number_of_valid_placeholders("%zd %zi %zu %zo %zx %zX %zn"), 7);
  EXPECT_EQ(count_number_of_valid_placeholders("%td %ti %tu %to %tx %tX %tn"), 7);
  EXPECT_EQ(count_number_of_valid_placeholders("%Lf %LF %Le %LE %Lg %LG %La %LA"), 8);

  // Invalid lengths
  EXPECT_EQ(count_number_of_valid_placeholders("%hhf %hhF %hhe %hhE %hhg %hhG %hha %hhA %hhc %hhs %hhp"), 0);
  EXPECT_EQ(count_number_of_valid_placeholders("%hf %hF %he %hE %hg %hG %ha %hA %hc %hs %hp"), 0);
  EXPECT_EQ(count_number_of_valid_placeholders("%lf %lF %le %lE %lg %lG %la %lA %lp"), 0);
  EXPECT_EQ(count_number_of_valid_placeholders("%llf %llF %lle %llE %llg %llG %lla %llA %llc %lls %llp"), 0);
  EXPECT_EQ(count_number_of_valid_placeholders("%jf %jF %je %jE %jg %jG %ja %jA %jc %js %jp"), 0);
  EXPECT_EQ(count_number_of_valid_placeholders("%zf %zF %ze %zE %zg %zG %za %zA %zc %zs %zp"), 0);
  EXPECT_EQ(count_number_of_valid_placeholders("%tf %tF %te %tE %tg %tG %ta %tA %tc %ts %tp"), 0);
  EXPECT_EQ(count_number_of_valid_placeholders("%Ld %Li %Lu %Lo %Lx %LX %Lc %Ls %Lp %Ln"), 0);
}

TEST_F(PlaceholderCounting, AllOptionalSpecifications) {
  EXPECT_EQ(count_number_of_valid_placeholders("%-9.3lld"), 1);
  EXPECT_EQ(count_number_of_valid_placeholders("%+*.3LA"), 2);
  EXPECT_EQ(count_number_of_valid_placeholders("%#1.*hhn"), 2);
  EXPECT_EQ(count_number_of_valid_placeholders("%0*.*jo"), 3);
}

TEST(PlaceholderMatching, UnsignedIntMatching) {
  EXPECT_TRUE(matcher::UnsignedIntType{}.matches(unsigned{}));
  EXPECT_TRUE(matcher::UnsignedIntType{}.matches(uint32_t{}));
  EXPECT_FALSE(matcher::UnsignedIntType{}.matches(int{}));
  EXPECT_FALSE(matcher::UnsignedIntType{}.matches(float{}));
}

TEST(PlaceholderMatching, SignedIntMatching) {
  EXPECT_TRUE(matcher::SignedIntType{}.matches(long{}));
  EXPECT_TRUE(matcher::SignedIntType{}.matches(int32_t{}));
  EXPECT_FALSE(matcher::SignedIntType{}.matches(unsigned{}));
  EXPECT_FALSE(matcher::SignedIntType{}.matches(float{}));
}

TEST(PlaceholderMatching, FloatMatching) {
  EXPECT_TRUE(matcher::FloatingType{}.matches(double{}));
  EXPECT_TRUE(matcher::FloatingType{}.matches(float{}));
  EXPECT_FALSE(matcher::FloatingType{}.matches(unsigned{}));
  EXPECT_FALSE(matcher::FloatingType{}.matches(int{}));
}

TEST(PlaceholderMatching, CharMatching) {
  EXPECT_TRUE(matcher::CharType{}.matches(char{}));
  EXPECT_TRUE(matcher::CharType{}.matches((unsigned char) {}));
  EXPECT_TRUE(matcher::CharType{}.matches(uint8_t{}));
  EXPECT_FALSE(matcher::CharType{}.matches((const char *) {}));
  EXPECT_FALSE(matcher::CharType{}.matches(int{}));
}

TEST(PlaceholderMatching, StringMatching) {
  EXPECT_TRUE(matcher::StringType{}.matches((const char *) {}));
  EXPECT_TRUE(matcher::StringType{}.matches(std::string{}));
  EXPECT_FALSE(matcher::StringType{}.matches(char{}));
  EXPECT_FALSE(matcher::StringType{}.matches((int *) {}));
}

TEST(PlaceholderMatching, PointerMatching) {
  EXPECT_TRUE(matcher::PointerType{}.matches((double *) {}));
  EXPECT_TRUE(matcher::PointerType{}.matches((const int *const) {}));
  EXPECT_TRUE(matcher::PointerType{}.matches((const char *) {}));
  EXPECT_FALSE(matcher::PointerType{}.matches(float{}));
  EXPECT_FALSE(matcher::PointerType{}.matches(int{}));
}

TEST(PlaceholderMatching, ParseWidthSpecifier) {
  const auto result = parse_format_to_placeholder_matchers("%*f");
  ASSERT_EQ(result.size(), 2);
  EXPECT_TRUE(result.at(0).matches<unsigned int>());
  EXPECT_FALSE(result.at(0).matches<float>());
  EXPECT_FALSE(result.at(0).matches<const char *>());
}

TEST(PlaceholderMatching, ParsePrecisionSpecifier) {
  const auto result = parse_format_to_placeholder_matchers("%.*f");
  ASSERT_EQ(result.size(), 2);
  EXPECT_TRUE(result.at(0).matches<unsigned int>());
  EXPECT_FALSE(result.at(0).matches<float>());
  EXPECT_FALSE(result.at(0).matches<const char *>());
}

TEST(Devel, Leved) {
  const auto result = parse_format_to_placeholder_matchers("%*.*f");
  EXPECT_EQ(result.size(), 3);
  EXPECT_TRUE(result.at(2).matches<float>());
  const auto x = result.at(2).get<log4tiny::matcher::FloatingType>();
  EXPECT_EQ(x.flag.option, matcher::PlaceholderFlag::Option::FORCE_HEX_PREFIX);
  EXPECT_EQ(x.width, 1);
  EXPECT_EQ(x.precision, 1);
}
