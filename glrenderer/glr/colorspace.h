#pragma once
#include <tuple>

namespace glr {

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
