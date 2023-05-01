#include <gtest/gtest.h>
#include <vrm/gltfon.h>

TEST(gltfon, literal)
{
  {
    auto result = gltfon::Parse(" null ");
    EXPECT_TRUE(result->IsNull());
  }
  {
    auto result = gltfon::Parse(" true ");
    EXPECT_TRUE(result->IsTrue());
  }
  {
    auto result = gltfon::Parse(" false ");
    EXPECT_TRUE(result->IsFalse());
  }
}

TEST(gltfon, number)
{
  {
    auto result = gltfon::Parse(" 123 ");
    EXPECT_EQ(result->Number<int>(), 123);
  }
}

TEST(gltfon, string)
{
  {
    auto result = gltfon::Parse(" \"a\" ");
    EXPECT_EQ(result->Unquoted(), "a");
  }
}

TEST(gltfon, array)
{
  // {
  //   auto result = gltfon::Parse(" [] ");
  //   EXPECT_EQ(result->Size(), 0);
  // }
  //
  // {
  //   auto result = gltfon::Parse(" [1,2,3] ");
  //   EXPECT_EQ(result->Size(), 3);
  //   EXPECT_EQ(result->Get(0)->Number<int>(), 1);
  //   EXPECT_EQ(result->Get(1)->Number<int>(), 2);
  //   EXPECT_EQ(result->Get(2)->Number<int>(), 3);
  // }
}

TEST(gltfon, object) {}
