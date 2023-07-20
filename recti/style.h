#pragma once
#include "operation.h"
#include "vec4.h"
#include <stdint.h>

namespace recti {

enum COLOR
{
  DIRECTION_X,      // directionColor[0]
  DIRECTION_Y,      // directionColor[1]
  DIRECTION_Z,      // directionColor[2]
  PLANE_X,          // planeColor[0]
  PLANE_Y,          // planeColor[1]
  PLANE_Z,          // planeColor[2]
  SELECTION,        // selectionColor
  INACTIVE,         // inactiveColor
  TRANSLATION_LINE, // translationLineColor
  SCALE_LINE,
  ROTATION_USING_BORDER,
  ROTATION_USING_FILL,
  HATCHED_AXIS_LINES,
  TEXT,
  TEXT_SHADOW,
  COUNT
};

struct Style
{
  // Thickness of lines for translation gizmo
  float TranslationLineThickness = 3.0f;

  // Size of arrow at the end of lines for translation gizmo
  float TranslationLineArrowSize = 6.0f;

  // Thickness of lines for rotation gizmo
  float RotationLineThickness = 2.0f;

  // Thickness of line surrounding the rotation gizmo
  float RotationOuterLineThickness = 3.0f;

  // Thickness of lines for scale gizmo
  float ScaleLineThickness = 3.0f;

  // Size of circle at the end of lines for scale gizmo
  float ScaleLineCircleSize = 6.0f;

  // Thickness of hatched axis lines
  float HatchedAxisLineThickness = 6.0f;

  // Size of circle at the center of the translate/scale gizmo
  float CenterCircleSize = 6.0f;

  recti::Vec4 Colors[COLOR::COUNT] = {
    { 0.666f, 0.000f, 0.000f, 1.000f }, { 0.000f, 0.666f, 0.000f, 1.000f },
    { 0.000f, 0.000f, 0.666f, 1.000f }, { 0.666f, 0.000f, 0.000f, 0.380f },
    { 0.000f, 0.666f, 0.000f, 0.380f }, { 0.000f, 0.000f, 0.666f, 0.380f },
    { 1.000f, 0.500f, 0.062f, 0.541f }, { 0.600f, 0.600f, 0.600f, 0.600f },
    { 0.666f, 0.666f, 0.666f, 0.666f }, { 0.250f, 0.250f, 0.250f, 1.000f },
    { 1.000f, 0.500f, 0.062f, 1.000f }, { 1.000f, 0.500f, 0.062f, 0.500f },
    { 0.000f, 0.000f, 0.000f, 0.500f }, { 1.000f, 1.000f, 1.000f, 1.000f },
    { 0.000f, 0.000f, 0.000f, 1.000f },
  };

  uint32_t GetColorU32(int idx) const;
};

uint32_t
ColorConvertFloat4ToU32(const Vec4& in);

uint32_t
COL32(uint8_t R, uint8_t G, uint8_t B, uint8_t A);

inline uint32_t
COL32_WHITE()
{
  return ColorConvertFloat4ToU32({ 1, 1, 1, 1 });
}
inline uint32_t
COL32_BLACK()
{
  return ColorConvertFloat4ToU32({ 0, 0, 0, 1 });
}

} // namespace
