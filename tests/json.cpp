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

TEST(JsonStream, write_array_0)
{
  std::stringstream ss;
  jsons::WriteFunc callback = [&ss](std::string_view str) { ss << str; };
  jsons::Writer writer(callback);

  writer.array_open();
  writer.array_close();
  auto str = ss.str();
  EXPECT_EQ(str, "[]");
}

TEST(JsonStream, write_array_1)
{
  std::stringstream ss;
  jsons::WriteFunc callback = [&ss](std::string_view str) { ss << str; };
  jsons::Writer writer(callback);

  writer.array_open();
  writer.write(1);
  writer.array_close();
  auto str = ss.str();
  EXPECT_EQ(str, "[1]");
}

TEST(JsonStream, write_array_2)
{
  std::stringstream ss;
  jsons::WriteFunc callback = [&ss](std::string_view str) { ss << str; };
  jsons::Writer writer(callback);

  writer.array_open();
  writer.write(1);
  writer.write(2);
  writer.array_close();
  auto str = ss.str();
  EXPECT_EQ(str, "[1,2]");
}

TEST(JsonStream, write_object_2)
{
  std::stringstream ss;
  jsons::WriteFunc callback = [&ss](std::string_view str) { ss << str; };
  jsons::Writer writer(callback);

  writer.object_open();
  writer.key("some");
  writer.write(1);
  writer.key("hoge");
  writer.write(2);
  writer.object_close();
  auto str = ss.str();
  EXPECT_EQ(str, "{\"some\":1,\"hoge\":2}");
}

TEST(JsonStream, write_object_nested)
{
  std::stringstream ss;
  jsons::WriteFunc callback = [&ss](std::string_view str) { ss << str; };
  jsons::Writer writer(callback);

  writer.object_open();
  writer.key("some");
  {
    writer.object_open();
    writer.key("hoge");
    writer.write(12);
    writer.object_close();
  }
  writer.object_close();
  auto str = ss.str();
  EXPECT_EQ(str, "{\"some\":{\"hoge\":12}}");
}