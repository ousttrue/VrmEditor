#include "drawcommand.h"
#include "mat4.h"
#include "model_context.h"
#include "style.h"

namespace recti {

void
DrawList::DrawHatchedAxis(const ModelContext& mCurrent,
                          const recti::Vec4& axis,
                          const Style& mStyle)
{
  if (mStyle.HatchedAxisLineThickness <= 0.0f) {
    return;
  }

  for (int j = 1; j < 10; j++) {
    recti::Vec2 baseSSpace2 =
      recti::worldToPos(axis * 0.05f * (float)(j * 2) * mCurrent.mScreenFactor,
                        mCurrent.mMVP,
                        mCurrent.mCameraMouse.Camera.Viewport);
    recti::Vec2 worldDirSSpace2 = recti::worldToPos(
      axis * 0.05f * (float)(j * 2 + 1) * mCurrent.mScreenFactor,
      mCurrent.mMVP,
      mCurrent.mCameraMouse.Camera.Viewport);
    AddLine(baseSSpace2,
            worldDirSSpace2,
            mStyle.GetColorU32(HATCHED_AXIS_LINES),
            mStyle.HatchedAxisLineThickness);
  }
}

} // namespace
