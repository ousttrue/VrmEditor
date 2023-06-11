#include <gtest/gtest.h>
#include <cctype>
#include <regex>

TEST(Text, isspace)
{
  EXPECT_TRUE(std::isspace('\n'));
}

TEST(Text, regex)
{
  std::cmatch match;
  auto m = std::regex_search("abc", match, std::regex("b"));
  EXPECT_TRUE(m);
}
