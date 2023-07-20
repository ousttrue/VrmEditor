static MOVETYPE
GetUniformType(const recti::ModelContext& mCurrent, bool mAllowAxisFlip)
{
  auto& mousePos = mCurrent.CameraMouse.Mouse.Position;
  recti::Vec4 deltaScreen = { mousePos.x - mCurrent.ScreenSquareCenter.x,
                              mousePos.y - mCurrent.ScreenSquareCenter.y,
                              0.f,
                              0.f };
  float dist = deltaScreen.Length();
  if (Contains(mCurrent.Operation, recti::SCALEU) && dist >= 17.0f &&
      dist < 23.0f) {
    return recti::MT_SCALE_XYZ;
  }

  for (int i = 0; i < 3; i++) {
    if (!Intersects(mCurrent.Operation,
                    static_cast<recti::OPERATION>(recti::SCALE_XU << i))) {
      continue;
    }

    recti::Tripod tripod(i);
    tripod.ComputeTripodAxisAndVisibility(mCurrent, mAllowAxisFlip);
    tripod.dirAxis.TransformVector(mCurrent.ModelLocal);
    tripod.dirPlaneX.TransformVector(mCurrent.ModelLocal);
    tripod.dirPlaneY.TransformVector(mCurrent.ModelLocal);

    // draw axis
    if (tripod.belowAxisLimit) {
      bool hasTranslateOnAxis =
        Contains(mCurrent.Operation,
                 static_cast<recti::OPERATION>(recti::TRANSLATE_X << i));
      float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
      recti::Vec2 worldDirSSpace = recti::worldToPos(
        (tripod.dirAxis * markerScale) * mCurrent.ScreenFactor,
        mCurrent.MVPLocal,
        mCurrent.CameraMouse.Camera.Viewport);

      float distance = sqrtf((worldDirSSpace - mousePos).SqrLength());
      if (distance < 12.f) {
        return (recti::MOVETYPE)(recti::MT_SCALE_X + i);
      }
    }
  }

  return MT_NONE;
}

MOVETYPE
UniformScaleGizmo::Hover(const ModelContext& current)
{
  if (!Intersects(current.Operation, SCALEU)) {
    return MT_NONE;
  }
  return GetUniformType(current, m_allowAxisFlip);
}

void
UniformScaleGizmo::Draw(const ModelContext& mCurrent,
                        MOVETYPE active,
                        MOVETYPE hover,
                        const Style& mStyle,
                        DrawList& drawList)
{
  // colors
  uint32_t colors[7];
  ComputeColors(colors, hover, mStyle);

  // draw
  Vec4 scaleDisplay = { 1.f, 1.f, 1.f, 1.f };

  for (int i = 0; i < 3; i++) {
    if (!Intersects(mCurrent.Operation,
                    static_cast<OPERATION>(SCALE_XU << i))) {
      continue;
    }
    const bool usingAxis = (active == MT_NONE || active == MT_SCALE_X + i);
    if (usingAxis) {
      Tripod tripod(i);
      tripod.ComputeTripodAxisAndVisibility(mCurrent, m_allowAxisFlip);
      tripod.dirAxis.TransformVector(mCurrent.ModelLocal);
      tripod.dirPlaneX.TransformVector(mCurrent.ModelLocal);
      tripod.dirPlaneY.TransformVector(mCurrent.ModelLocal);

      // draw axis
      if (tripod.belowAxisLimit) {
        bool hasTranslateOnAxis = Contains(
          mCurrent.Operation, static_cast<OPERATION>(TRANSLATE_X << i));
        float markerScale = hasTranslateOnAxis ? 1.4f : 1.0f;
        Vec2 worldDirSSpace =
          worldToPos((tripod.dirAxis * markerScale * scaleDisplay[i]) *
                       mCurrent.ScreenFactor,
                     mCurrent.MVPLocal,
                     mCurrent.CameraMouse.Camera.Viewport);

        drawList.AddCircleFilled(worldDirSSpace, 12.f, colors[i + 1]);
      }
    }
  }

  // draw screen cirle
  drawList.AddCircle(
    mCurrent.ScreenSquareCenter, 20.f, colors[0], 32, mStyle.CenterCircleSize);
}

//
// UniformScaleDragHandle
//
// ScaleUDragHandle::ScaleUDragHandle(const ModelContext& mCurrent, MOVETYPE type)
//   : ScaleDragHandle(mCurrent, type)
// {
// }
//
// bool
// ScaleUDragHandle::Drag(const ModelContext& mCurrent,
//                        const float* snap,
//                        float* matrix,
//                        float* deltaMatrix)
// {
//   {
//     float scaleDelta =
//       (mCurrent.CameraMouse.Mouse.Position.x - mSaveMousePosx) * 0.01f;
//     mScale.Set(max(1.f + scaleDelta, 0.001f));
//   }
//
//   return false;
// }
//
// void
// ScaleUDragHandle::Draw(const ModelContext& mCurrent,
//                        const Style& mStyle,
//                        DrawList& drawList)
// {
//
//   Vec2 destinationPosOnScreen =
//     mCurrent.CameraMouse.WorldToPos(mCurrent.Model.position());
//
//   char tmps[512];
//   int componentInfoIndex = (m_type - MT_SCALE_X) * 3;
//   snprintf(tmps,
//            sizeof(tmps),
//            scaleInfoMask[m_type - MT_SCALE_X],
//            mScale[translationInfoIndex[componentInfoIndex]]);
//   drawList.AddText(
//     Vec2(destinationPosOnScreen.x + 15, destinationPosOnScreen.y + 15),
//     mStyle.GetColorU32(TEXT_SHADOW),
//     tmps);
//   drawList.AddText(
//     Vec2(destinationPosOnScreen.x + 14, destinationPosOnScreen.y + 14),
//     mStyle.GetColorU32(TEXT),
//     tmps);
//
//   // if (mState.Using(mCurrent.mActualID)) {
//   //   uint32_t scaleLineColor = mStyle.GetColorU32(SCALE_LINE);
//   //   drawList->AddLine(baseSSpace,
//   //                     worldDirSSpaceNoScale,
//   //                     scaleLineColor,
//   //                     mStyle.ScaleLineThickness);
//   //   drawList->AddCircleFilled(
//   //     worldDirSSpaceNoScale, mStyle.ScaleLineCircleSize, scaleLineColor);
//   // }
// }



