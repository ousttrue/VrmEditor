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
inline std::tuple<ColorSpace, const char*> ColorSpaceCombo[] = {
  { ColorSpace::sRGB, "sRGB" },
  { ColorSpace::Linear, "Linear" },
};

struct Texture
{
  std::string Name;
  gltfjson::format::Id Sampler;
  std::shared_ptr<Image> Source;
  // Determined from how it is used in material.
  // Only BaseColorTexture and EmissiveTexture are sRGB
  ColorSpace ColorSpace = ColorSpace::sRGB;
};

}
}
