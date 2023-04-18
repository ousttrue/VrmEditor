#pragma once
#include <memory>
#include <span>
#include <stb_image.h>
#include <string>
#include <string_view>
#include <vector>

namespace libvrm::gltf {
class Image
{

  std::string m_name;
  std::vector<uint8_t> m_pixels;
  int m_width = 0;
  int m_height = 0;
  int m_sourceChannels = 0;

public:
  Image(std::string_view name)
    : m_name(name)
  {
  }
  const std::string& Name() const { return m_name; }
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
      return true;
    } else {
      return false;
    }
  }
};

class Material
{
  std::string m_name;

public:
  Material(std::string_view name)
    : m_name(name)
  {
  }
  std::shared_ptr<Image> texture;
};
} // namespace gltf
