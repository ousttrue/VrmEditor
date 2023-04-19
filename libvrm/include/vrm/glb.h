#pragma once
#include <expected>
#include <filesystem>
#include <functional>
#include <optional>
#include <span>
#include <stdint.h>
#include <string>
#include <tuple>

namespace libvrm::gltf {
struct Glb
{
  std::span<const uint8_t> JsonChunk;
  uint32_t JsonPadding() const
  {
    auto json_padding = 0;
    if (JsonChunk.size() % 4) {
      json_padding = 4 - (JsonChunk.size() % 4);
    }
    return json_padding;
  }

  std::span<const uint8_t> BinChunk;
  uint32_t BinPadding() const
  {
    auto bin_padding = 0;
    if (BinChunk.size() % 4) {
      bin_padding = 4 - (BinChunk.size() % 4);
    }
    return bin_padding;
  }

  static std::optional<Glb> Parse(std::span<const uint8_t> bytes);
  uint32_t CalcSize() const
  {
    return
      // glb header
      12
      // json chunk
      + 8 + JsonChunk.size() +
      JsonPadding()
      // bin chunk
      + 8 + BinChunk.size() + BinPadding();
  }

  bool WriteTo(std::ostream &os);
};

}
