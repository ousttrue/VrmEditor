#pragma once
#include <memory>

namespace libvrm {
namespace gltf {

class Image;

enum class TextureMagFilter
{
  NEAREST = 9728,
  LINEAR = 9729,
};
enum class TextureMinFilter
{
  NEAREST = 9728,
  LINEAR = 9729,
  NEAREST_MIPMAP_NEAREST = 9984,
  LINEAR_MIPMAP_NEAREST = 9985,
  NEAREST_MIPMAP_LINEAR = 9986,
  LINEAR_MIPMAP_LINEAR = 9987,
};
enum class TextureWrap
{
  CLAMP_TO_EDGE = 33071,
  MIRRORED_REPEAT = 33648,
  REPEAT = 10497,
};

struct TextureSampler
{
  TextureMagFilter MagFilter = TextureMagFilter::NEAREST;
  TextureMinFilter MinFilter = TextureMinFilter::NEAREST;
  TextureWrap WrapS = TextureWrap::REPEAT;
  TextureWrap WrapT = TextureWrap::REPEAT;
};

enum class ColorSpace
{
  sRGB,
  Linear,
};

struct Texture
{
  std::shared_ptr<TextureSampler> Sampler;
  std::shared_ptr<Image> Source;
  // Determined from how it is used in material.
  // Only BaseColorTexture and EmissiveTexture are sRGB
  ColorSpace ColorSpace = ColorSpace::sRGB;
};

}
}
