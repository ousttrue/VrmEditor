#pragma once
#include "camera_mouse.h"
#include "operation.h"
#include <stdint.h>

namespace recti {

struct ModelContext
{
  int64_t ActualID;
  OPERATION Operation;
  MODE Mode;
  const CameraMouse& CameraMouse;
  Mat4 Model;
  Mat4 ModelLocal; // orthonormalized model
  Mat4 ModelInverse;
  Mat4 ModelSource;
  Mat4 ModelSourceInverse;
  Mat4 MVP;
  Mat4 MVPLocal; // MVP with full model matrix whereas mMVP's model matrix
                 // might only be translation in case of World space edition
  Vec4 ModelScaleOrigin;
  float ScreenFactor;

  // window coords
  Vec2 ScreenSquareCenter;

  // ,
  ModelContext(int64_t actualID,
               OPERATION operation,
               MODE mode,
               const float* matrix,
               const struct CameraMouse& cameraMouse,
               float gizmoSizeClipSpace)
    : ActualID(actualID)
    , Operation(operation)
    , Mode((operation & SCALE) ? LOCAL : mode)
    , CameraMouse(cameraMouse)
  {
    ModelLocal = *(Mat4*)matrix;
    ModelLocal.OrthoNormalize();

    if (Mode == LOCAL) {
      Model = ModelLocal;
    } else {
      Model.Translation(((Mat4*)matrix)->position());
    }
    ModelSource = *(Mat4*)matrix;
    ModelScaleOrigin.Set(ModelSource.right().Length(),
                         ModelSource.up().Length(),
                         ModelSource.dir().Length());

    ModelInverse.Inverse(Model);
    ModelSourceInverse.Inverse(ModelSource);
    MVP = Model * cameraMouse.mViewProjection;
    MVPLocal = ModelLocal * cameraMouse.mViewProjection;

    // compute scale from the size of camera right vector projected on screen at
    // the matrix position
    Vec4 pointRight = cameraMouse.mViewInverse.right();
    pointRight.TransformPoint(cameraMouse.mViewProjection);

    ScreenFactor = gizmoSizeClipSpace / (pointRight.x / pointRight.w -
                                         MVP.position().x / MVP.position().w);
    Vec4 rightViewInverse = cameraMouse.mViewInverse.right();
    rightViewInverse.TransformVector(ModelInverse);
    float rightLength = GetSegmentLengthClipSpace(
      { 0.f, 0.f }, rightViewInverse, MVP, cameraMouse.Camera.AspectRatio());
    ScreenFactor = gizmoSizeClipSpace / rightLength;

    ScreenSquareCenter = cameraMouse.WorldToPos(Model.position());
  }

  bool MouseInScreenSquare() const
  {
    return recti::IsWithin(CameraMouse.Mouse.Position.x,
                           ScreenSquareCenter.x - 10,
                           ScreenSquareCenter.x + 10) &&
           recti::IsWithin(CameraMouse.Mouse.Position.y,
                           ScreenSquareCenter.y - 10,
                           ScreenSquareCenter.y + 10);
  }
};

} // namespace
