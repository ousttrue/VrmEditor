#pragma once
#include <optional>
#include <span>
#include <stdint.h>
#include <string>
#include <vector>

namespace libvrm {
namespace gltf {

enum class ImageType
{
  Jpeg,
  Png,
};
struct EncodedImage
{
  ImageType Type;
  std::span<const uint8_t> Bytes;
};

class Image
{
  std::vector<uint8_t> m_pixels;
  int m_width = 0;
  int m_height = 0;
  int m_sourceChannels = 0;

public:
  static bool IsJpeg(std::span<const uint8_t> data);
  static bool IsPng(std::span<const uint8_t> data);
  std::string Type() const
  {
    if (Encoded) {
      switch (Encoded->Type) {
        case ImageType::Jpeg:
          return "jpg";

        case ImageType::Png:
          return "png";

        default:
          return "unknown";
      }
    } else {
      return "raw";
    }
  }
  std::string Name;
  std::optional<EncodedImage> Encoded;
  Image(std::string_view name);
  int Width() const { return m_width; }
  int Height() const { return m_height; }
  int Channels() const { return m_sourceChannels; }
  const uint8_t* Pixels() const
  {
    return m_pixels.empty() ? nullptr : m_pixels.data();
  }
  bool Load(std::span<const uint8_t> data);
  bool LoadHdr(std::span<const uint8_t> data);
};

}
}
