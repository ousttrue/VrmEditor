#include "style.h"
#include <assert.h>

namespace recti {

static inline float
ImSaturate(float f)
{
  return (f < 0.0f) ? 0.0f : (f > 1.0f) ? 1.0f : f;
}
#define IM_F32_TO_INT8_SAT(_VAL)                                               \
  ((int)(ImSaturate(_VAL) * 255.0f + 0.5f)) // Saturated, always output 0..255
uint32_t
ColorConvertFloat4ToU32(const recti::Vec4& in)
{
  uint32_t out;
  out = ((uint32_t)IM_F32_TO_INT8_SAT(in.x)) << 0;
  out |= ((uint32_t)IM_F32_TO_INT8_SAT(in.y)) << 8;
  out |= ((uint32_t)IM_F32_TO_INT8_SAT(in.z)) << 16;
  out |= ((uint32_t)IM_F32_TO_INT8_SAT(in.w)) << 24;
  return out;
}

#define IM_COL32_R_SHIFT 0
#define IM_COL32_G_SHIFT 8
#define IM_COL32_B_SHIFT 16
#define IM_COL32_A_SHIFT 24
#define IM_COL32_A_MASK 0xFF000000

uint32_t
COL32(uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
  return (
    ((uint32_t)(A) << IM_COL32_A_SHIFT) | ((uint32_t)(B) << IM_COL32_B_SHIFT) |
    ((uint32_t)(G) << IM_COL32_G_SHIFT) | ((uint32_t)(R) << IM_COL32_R_SHIFT));
}

uint32_t
Style::GetColorU32(int idx) const
{
  assert(idx < COLOR::COUNT);
  return ColorConvertFloat4ToU32(Colors[idx]);
}

void
Style::ComputeColors(uint32_t* colors, int type, OPERATION operation) const
{
  uint32_t selectionColor = GetColorU32(SELECTION);

  switch (operation) {
    case TRANSLATE:
      colors[0] = (type == MT_MOVE_SCREEN) ? selectionColor : COL32_WHITE();
      for (int i = 0; i < 3; i++) {
        colors[i + 1] = (type == (int)(MT_MOVE_X + i))
                          ? selectionColor
                          : GetColorU32(DIRECTION_X + i);
        colors[i + 4] = (type == (int)(MT_MOVE_YZ + i))
                          ? selectionColor
                          : GetColorU32(PLANE_X + i);
        colors[i + 4] =
          (type == MT_MOVE_SCREEN) ? selectionColor : colors[i + 4];
      }
      break;
    case ROTATE:
      colors[0] = (type == MT_ROTATE_SCREEN) ? selectionColor : COL32_WHITE();
      for (int i = 0; i < 3; i++) {
        colors[i + 1] = (type == (int)(MT_ROTATE_X + i))
                          ? selectionColor
                          : GetColorU32(DIRECTION_X + i);
      }
      break;
    case SCALEU:
    case SCALE:
      colors[0] = (type == MT_SCALE_XYZ) ? selectionColor : COL32_WHITE();
      for (int i = 0; i < 3; i++) {
        colors[i + 1] = (type == (int)(MT_SCALE_X + i))
                          ? selectionColor
                          : GetColorU32(DIRECTION_X + i);
      }
      break;
    // note: this internal function is only called with three possible
    // values for operation
    default:
      break;
  }
}

} // namespace
