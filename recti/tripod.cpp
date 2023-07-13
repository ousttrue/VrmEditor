#include "tripod.h"
namespace recti {

Tripod::Tripod(const recti::ModelContext& mCurrent,
               bool mAllowAxisFlip,
               const int axisIndex,
               recti::State* state)
{
  static const recti::Vec4 directionUnary[3] = { { 1.f, 0.f, 0.f, 0 },
                                                 { 0.f, 1.f, 0.f, 0 },
                                                 { 0.f, 0.f, 1.f, 0 } };

  dirAxis = directionUnary[axisIndex];
  dirPlaneX = directionUnary[(axisIndex + 1) % 3];
  dirPlaneY = directionUnary[(axisIndex + 2) % 3];

  if (state->Using(mCurrent.mActualID)) {
    // when using, use stored factors so the gizmo doesn't flip when we
    // translate
    belowAxisLimit = state->mBelowAxisLimit[axisIndex];
    belowPlaneLimit = state->mBelowPlaneLimit[axisIndex];
    dirAxis *= state->mAxisFactor[axisIndex];
    dirPlaneX *= state->mAxisFactor[(axisIndex + 1) % 3];
    dirPlaneY *= state->mAxisFactor[(axisIndex + 2) % 3];
    return;
  }

  // new method

  float lenDir =
    GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                              dirAxis,
                              mCurrent.mMVP,
                              mCurrent.mCameraMouse.Camera.DisplayRatio());
  float lenDirMinus =
    GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                              -dirAxis,
                              mCurrent.mMVP,
                              mCurrent.mCameraMouse.Camera.DisplayRatio());

  float lenDirPlaneX =
    GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                              dirPlaneX,
                              mCurrent.mMVP,
                              mCurrent.mCameraMouse.Camera.DisplayRatio());
  float lenDirMinusPlaneX =
    GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                              -dirPlaneX,
                              mCurrent.mMVP,
                              mCurrent.mCameraMouse.Camera.DisplayRatio());

  float lenDirPlaneY =
    GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                              dirPlaneY,
                              mCurrent.mMVP,
                              mCurrent.mCameraMouse.Camera.DisplayRatio());
  float lenDirMinusPlaneY =
    GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                              -dirPlaneY,
                              mCurrent.mMVP,
                              mCurrent.mCameraMouse.Camera.DisplayRatio());

  // For readability
  mulAxis = (mAllowAxisFlip && lenDir < lenDirMinus &&
             fabsf(lenDir - lenDirMinus) > FLT_EPSILON)
              ? -1.f
              : 1.f;
  mulAxisX = (mAllowAxisFlip && lenDirPlaneX < lenDirMinusPlaneX &&
              fabsf(lenDirPlaneX - lenDirMinusPlaneX) > FLT_EPSILON)
               ? -1.f
               : 1.f;
  mulAxisY = (mAllowAxisFlip && lenDirPlaneY < lenDirMinusPlaneY &&
              fabsf(lenDirPlaneY - lenDirMinusPlaneY) > FLT_EPSILON)
               ? -1.f
               : 1.f;
  dirAxis *= mulAxis;
  dirPlaneX *= mulAxisX;
  dirPlaneY *= mulAxisY;

  // for axis
  float axisLengthInClipSpace =
    GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                              dirAxis * mCurrent.mScreenFactor,
                              mCurrent.mMVP,
                              mCurrent.mCameraMouse.Camera.DisplayRatio());

  float paraSurf =
    GetParallelogram({ 0.f, 0.f, 0.f },
                     dirPlaneX * mCurrent.mScreenFactor,
                     dirPlaneY * mCurrent.mScreenFactor,
                     mCurrent.mMVP,
                     mCurrent.mCameraMouse.Camera.DisplayRatio());
  belowPlaneLimit = (paraSurf > 0.0025f);
  belowAxisLimit = (axisLengthInClipSpace > 0.02f);

  // and store values
  state->mAxisFactor[axisIndex] = mulAxis;
  state->mAxisFactor[(axisIndex + 1) % 3] = mulAxisX;
  state->mAxisFactor[(axisIndex + 2) % 3] = mulAxisY;
  state->mBelowAxisLimit[axisIndex] = belowAxisLimit;
  state->mBelowPlaneLimit[axisIndex] = belowPlaneLimit;
}

}
