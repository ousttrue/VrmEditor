#include "image.h"
#include <assert.h>
#include <stb_image.h>

namespace libvrm {

bool
Image::IsJpeg(std::span<const uint8_t> data)
{
  if(data.size()<2)
  {
    return false;
  }
  static const uint8_t SOI[]{ 0xFF, 0xD8 };
  for(int i=0; i<2; ++i)
  {
    if(data[i]!=SOI[i])
    {
      return false;
    }
  }
  return true;
}

bool
Image::IsPng(std::span<const uint8_t> data)
{
  static const uint8_t PNG[]{
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
  };
  if(data.size()<8)
  {
    return false;
  }
  for(int i=0; i<8; ++i)
  {
    if(data[i]!=PNG[i])
    {
      return false;
    }
  }
  return true;
}

bool
Image::IsGif(std::span<const uint8_t> data)
{
  if (data.size() < 3) {
    return false;
  }
  if (data[0] != 'G') {
    return false;
  }
  if (data[1] != 'I') {
    return false;
  }
  if (data[2] != 'F') {
    return false;
  }
  return true;
}

std::string Name;
std::optional<EncodedImage> Encoded;
Image::Image(std::string_view name)
  : Name(name)
{
}
bool
Image::Load(std::span<const uint8_t> data)
{
  if (auto pixels = stbi_load_from_memory(
        data.data(), data.size(), &m_width, &m_height, &m_sourceChannels, 4)) {
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
    } else if (IsGif(data)) {
      Encoded = EncodedImage{
        .Type = ImageType::Gif,
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

bool
Image::LoadHdr(std::span<const uint8_t> data)
{
  stbi_set_flip_vertically_on_load(true);

  auto pixels = (uint8_t*)stbi_loadf_from_memory(
    data.data(), data.size(), &m_width, &m_height, &m_sourceChannels, 0);
  stbi_set_flip_vertically_on_load(false);
  assert(m_sourceChannels == 3);
  if (pixels) {
    m_pixels.assign(pixels, pixels + m_width * m_height * 12);
    stbi_image_free(pixels);

    return true;
  } else {
    return false;
  }
}

} // namespace
