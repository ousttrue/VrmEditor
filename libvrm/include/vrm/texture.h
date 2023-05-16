#pragma once
#include <gltfjson/gltf.h>
#include <memory>

namespace libvrm {
namespace gltf {

class Image;

enum class ColorSpace
{
  sRGB,
  Linear,
};

struct Texture
{
  std::shared_ptr<gltfjson::format::Sampler> Sampler;
  std::shared_ptr<Image> Source;
  // Determined from how it is used in material.
  // Only BaseColorTexture and EmissiveTexture are sRGB
  ColorSpace ColorSpace = ColorSpace::sRGB;
};

}
}
