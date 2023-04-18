#pragma once
#include <assert.h>
#include <memory>
#include <span>
#include <stb_image.h>
#include <string>
#include <string_view>
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
  static bool IsJpeg(std::span<const uint8_t> data)
  {
    static const uint8_t SOI[]{ 0xFF, 0xD8 };
    return *((uint16_t*)data.data()) == *((uint16_t*)SOI);
  }

  static bool IsPng(std::span<const uint8_t> data)
  {
    static const uint8_t PNG[]{
      0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
    };
    return *((uint64_t*)data.data()) == *((uint64_t*)PNG);
  }
  std::string Name;
  std::optional<EncodedImage> Encoded;
  Image(std::string_view name)
    : Name(name)
  {
  }
  int Width() const { return m_width; }
  int Height() const { return m_height; }
  const uint8_t* Pixels() const
  {
    return m_pixels.empty() ? nullptr : m_pixels.data();
  }
  bool Load(std::span<const uint8_t> data)
  {
    if (auto pixels = stbi_load_from_memory(data.data(),
                                            data.size(),
                                            &m_width,
                                            &m_height,
                                            &m_sourceChannels,
                                            4)) {
      m_pixels.assign(pixels, pixels + m_width * m_height * 4);
      stbi_image_free(pixels);

      if (IsJpeg(data)) {
        Encoded = EncodedImage{
          .Type = ImageType::Jpeg,
          .Bytes = data,
        };
      } else if (IsPng(data)) {
        Encoded = EncodedImage{
          .Type = ImageType::Png,
          .Bytes = data,
        };
      } else {
        assert(false);
      }

      return true;
    } else {
      return false;
    }
  }
};

struct TextureSampler
{};

struct Texture
{
  std::shared_ptr<TextureSampler> Sampler;
  std::shared_ptr<Image> Source;
};

}
}
