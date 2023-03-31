#include <gtest/gtest.h>
#include <stdint.h>
#include <string>
#include <string_view>
#include <vrm/base64.h>

auto DECODED = "ABCDEFG";
auto ENCODED = "QUJDREVGRw==";

TEST(Base64, Encode) {
  auto encoded = gltf::Encode(DECODED);
  EXPECT_EQ(encoded, ENCODED);
}

TEST(Base64, Decode) {
  auto decoded = gltf::Decode(ENCODED);
  EXPECT_EQ(std::string((char *)decoded.data(), decoded.size()), DECODED);
}
