#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <vrm/jsons.h>

template<typename T>
static void
EQ(T value)
{
  std::stringstream ss;
  jsons::WriteFunc callback = [&ss](std::string_view str) { ss << str; };
  jsons::Writer writer(callback);
  writer.write(value);
  auto str = ss.str();
  auto parsed = nlohmann::json::parse(str);
  EXPECT_EQ(parsed, value);
}

TEST(JsonStream, write_null)
{
  std::stringstream ss;
  jsons::WriteFunc callback = [&ss](std::string_view str) { ss << str; };
  jsons::Writer writer(callback);
  writer.write_null();

  auto parsed = nlohmann::json::parse(ss.str());
  EXPECT_TRUE(parsed.is_null());
}

TEST(JsonStream, write_bool)
{
  EQ(true);
  EQ(false);
}

TEST(JsonStream, write_int)
{
  EQ(1);
  EQ(10);
}

TEST(JsonStream, write_short)
{
  EQ((short)1);
  EQ((short)10);
}

TEST(JsonStream, write_float)
{
  EQ(1.0f);
  EQ(10.0f);
}

TEST(JsonStream, write_string)
{
  EQ("hoge");
  // escape
  // utf-8
}
