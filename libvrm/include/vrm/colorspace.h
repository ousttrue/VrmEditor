#pragma once
#include <tuple>

namespace libvrm {
namespace gltf {

enum class ColorSpace
{
  sRGB,
  Linear,
};
inline std::tuple<ColorSpace, const char*> ColorSpaceCombo[] = {
  { ColorSpace::sRGB, "sRGB" },
  { ColorSpace::Linear, "Linear" },
};

}
}
