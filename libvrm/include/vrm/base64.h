#pragma once
#include <assert.h>
#include <span>
#include <stdint.h>
#include <string>
#include <string_view>
#include <vector>

namespace gltf {
//
static char EncodeMap[64]{
    'A', // 0b000000
    'B', // 0b000001
    'C', // 0b000010
    'D', // 0b000011
    'E', // 0b000100
    'F', // 0b000101
    'G', // 0b000110
    'H', // 0b000111
    'I', // 0b001000
    'J', // 0b001001
    'K', // 0b001010
    'L', // 0b001011
    'M', // 0b001100
    'N', // 0b001101
    'O', // 0b001110
    'P', // 0b001111
    'Q', // 0b010000
    'R', // 0b010001
    'S', // 0b010010
    'T', // 0b010011
    'U', // 0b010100
    'V', // 0b010101
    'W', // 0b010110
    'X', // 0b010111
    'Y', // 0b011000
    'Z', // 0b011001
    'a', // 0b011010
    'b', // 0b011011
    'c', // 0b011100
    'd', // 0b011101
    'e', // 0b011110
    'f', // 0b011111
    'g', // 0b100000
    'h', // 0b100001
    'i', // 0b100010
    'j', // 0b100011
    'k', // 0b100100
    'l', // 0b100101
    'm', // 0b100110
    'n', // 0b100111
    'o', // 0b101000
    'p', // 0b101001
    'q', // 0b101010
    'r', // 0b101011
    's', // 0b101100
    't', // 0b101101
    'u', // 0b101110
    'v', // 0b101111
    'w', // 0b110000
    'x', // 0b110001
    'y', // 0b110010
    'z', // 0b110011
    '0', // 0b110100
    '1', // 0b110101
    '2', // 0b110110
    '3', // 0b110111
    '4', // 0b111000
    '5', // 0b111001
    '6', // 0b111010
    '7', // 0b111011
    '8', // 0b111100
    '9', // 0b111101
    '0', // 0b111110
    '1', // 0b111111
};

inline uint32_t pack3(uint8_t c0, uint8_t c1, uint8_t c2) {
  return ((uint32_t)c0 << 16) + ((uint32_t)c1 << 8) + uint32_t(c2);
}

inline std::string Encode(std::span<const uint8_t> src) {

  auto size3 = src.size() / 3 * 3;
  auto remain = src.size() % 3;
  std::vector<char> encoded;
  encoded.reserve(size3 + remain ? 1 : 0);

  int i = 0;
  for (; i < size3; i += 3) {
    auto x = pack3(src[i], src[i + 1], src[i + 2]);
    encoded.push_back(EncodeMap[(x >> 18) & 0b111111]);
    encoded.push_back(EncodeMap[(x >> 12) & 0b111111]);
    encoded.push_back(EncodeMap[(x >> 6) & 0b111111]);
    encoded.push_back(EncodeMap[x & 0b111111]);
  }

  if (remain == 0) {
    // enced
    assert(i == src.size());
  } else if (remain == 1) {
    auto x = pack3(src[i], 0, 0);
    encoded.push_back(EncodeMap[(x >> 18) & 0b111111]);
    encoded.push_back(EncodeMap[(x >> 12) & 0b111111]);
    encoded.push_back('=');
    encoded.push_back('=');
  } else if (remain == 2) {
    auto x = pack3(src[i], src[i + 1], 0);
    encoded.push_back(EncodeMap[(x >> 18) & 0b111111]);
    encoded.push_back(EncodeMap[(x >> 12) & 0b111111]);
    encoded.push_back(EncodeMap[(x >> 6) & 0b111111]);
    encoded.push_back('=');
  }
  return {encoded.begin(), encoded.end()};
}

inline std::string Encode(std::string_view src) {
  return Encode(std::span{(const uint8_t *)src.data(), src.size()});
}

inline uint32_t base64_to_6bit(int c) {
  if (c == '=')
    return 0;
  if (c == '/')
    return 63;
  if (c == '+')
    return 62;
  if (c <= '9')
    return (c - '0') + 52;
  if ('a' <= c)
    return (c - 'a') + 26;
  return (c - 'A');
}

inline std::vector<uint8_t> Decode(std::string_view src) {
  std::vector<uint8_t> decoded;
  decoded.reserve(src.size() / 4 * 3);
  for (int i = 0; i < src.size(); i += 4) {
    auto o0 = base64_to_6bit(src[i]);
    auto o1 = base64_to_6bit(src[i + 1]);
    auto o2 = base64_to_6bit(src[i + 2]);
    auto o3 = base64_to_6bit(src[i + 3]);
    decoded.push_back((o0 << 2) | ((o1 & 0x30) >> 4));
    decoded.push_back(((o1 & 0xf) << 4) | ((o2 & 0x3c) >> 2));
    decoded.push_back(((o2 & 0x3) << 6) | (o3 & 0x3f));
  }
  if (src[src.size() - 1] == '=') {
    decoded.pop_back();
  }
  if (src[src.size() - 2] == '=') {
    decoded.pop_back();
  }
  return decoded;
}

} // namespace gltf
