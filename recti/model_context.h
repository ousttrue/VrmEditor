#pragma once
#include "camera_mouse.h"
#include "operation.h"
#include <stdint.h>

namespace recti {

struct ModelContext
{
  int64_t mActualID;
  OPERATION mOperation;
  MODE mMode;
  const CameraMouse& mCameraMouse;
  Mat4 mModel;
  Mat4 mModelLocal; // orthonormalized model
  Mat4 mModelInverse;
  Mat4 mModelSource;
  Mat4 mModelSourceInverse;
  Mat4 mMVP;
  Mat4 mMVPLocal; // MVP with full model matrix whereas mMVP's model matrix
                  // might only be translation in case of World space edition
  Vec4 mModelScaleOrigin;
  float mScreenFactor;

  // window coords
  Vec2 mScreenSquareCenter;

  // ,
  ModelContext(int64_t actualID,
               OPERATION operation,
               MODE mode,
               const float* matrix,
               const CameraMouse& cameraMouse,
               float gizmoSizeClipSpace)
    : mActualID(actualID)
    , mOperation(operation)
    , mMode((operation & SCALE) ? LOCAL : mode)
    , mCameraMouse(cameraMouse)
  {
    mModelLocal = *(Mat4*)matrix;
    mModelLocal.OrthoNormalize();

    if (mMode == LOCAL) {
      mModel = mModelLocal;
    } else {
      mModel.Translation(((Mat4*)matrix)->position());
    }
    mModelSource = *(Mat4*)matrix;
    mModelScaleOrigin.Set(mModelSource.right().Length(),
                          mModelSource.up().Length(),
                          mModelSource.dir().Length());

    mModelInverse.Inverse(mModel);
    mModelSourceInverse.Inverse(mModelSource);
    mMVP = mModel * cameraMouse.mViewProjection;
    mMVPLocal = mModelLocal * cameraMouse.mViewProjection;

    // compute scale from the size of camera right vector projected on screen at
    // the matrix position
    Vec4 pointRight = cameraMouse.mViewInverse.right();
    pointRight.TransformPoint(cameraMouse.mViewProjection);

    mScreenFactor =
      gizmoSizeClipSpace / (pointRight.x / pointRight.w -
                            this->mMVP.position().x / this->mMVP.position().w);
    Vec4 rightViewInverse = cameraMouse.mViewInverse.right();
    rightViewInverse.TransformVector(this->mModelInverse);
    float rightLength = GetSegmentLengthClipSpace(
      { 0.f, 0.f }, rightViewInverse, mMVP, cameraMouse.Camera.DisplayRatio());
    mScreenFactor = gizmoSizeClipSpace / rightLength;

    mScreenSquareCenter = cameraMouse.WorldToPos(mModel.position());
  }
};

} // namespace
