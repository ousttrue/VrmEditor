#include <gtest/gtest.h>
#include <cctype>
TEST(Text, isspace)
{
  EXPECT_TRUE(std::isspace('\n'));
}

