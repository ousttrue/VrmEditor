#include "tripod.h"
namespace recti {

Tripod::Tripod(const int axisIndex)
{
  // new method
  dirAxis = Vec4::DirectionUnary[axisIndex];
  dirPlaneX = Vec4::DirectionUnary[(axisIndex + 1) % 3];
  dirPlaneY = Vec4::DirectionUnary[(axisIndex + 2) % 3];
}

void
Tripod::ComputeTripodAxisAndVisibility(const recti::ModelContext& mCurrent,
                                       bool mAllowAxisFlip)
{
  float lenDir =
    GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                              dirAxis,
                              mCurrent.MVP,
                              mCurrent.CameraMouse.Camera.DisplayRatio());
  float lenDirMinus =
    GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                              -dirAxis,
                              mCurrent.MVP,
                              mCurrent.CameraMouse.Camera.DisplayRatio());

  float lenDirPlaneX =
    GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                              dirPlaneX,
                              mCurrent.MVP,
                              mCurrent.CameraMouse.Camera.DisplayRatio());
  float lenDirMinusPlaneX =
    GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                              -dirPlaneX,
                              mCurrent.MVP,
                              mCurrent.CameraMouse.Camera.DisplayRatio());

  float lenDirPlaneY =
    GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                              dirPlaneY,
                              mCurrent.MVP,
                              mCurrent.CameraMouse.Camera.DisplayRatio());
  float lenDirMinusPlaneY =
    GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                              -dirPlaneY,
                              mCurrent.MVP,
                              mCurrent.CameraMouse.Camera.DisplayRatio());

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

  // axis is visible ?
  float axisLengthInClipSpace =
    GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f },
                              dirAxis * mCurrent.ScreenFactor,
                              mCurrent.MVP,
                              mCurrent.CameraMouse.Camera.DisplayRatio());
  belowAxisLimit = (axisLengthInClipSpace > 0.02f);

  // plane is visible ?
  float paraSurf = GetParallelogram({ 0.f, 0.f, 0.f },
                                    dirPlaneX * mCurrent.ScreenFactor,
                                    dirPlaneY * mCurrent.ScreenFactor,
                                    mCurrent.MVP,
                                    mCurrent.CameraMouse.Camera.DisplayRatio());
  belowPlaneLimit = (paraSurf > 0.0025f);
}

}
