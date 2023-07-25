#include "tripod.h"
namespace recti {

Tripod::Tripod(const Mat4& mvp,
               float displayRatio,
               float screenFactor,
               bool mAllowAxisFlip,
               int axisIndex)
{
  {
    dirAxis = Vec4::DirectionUnary[axisIndex];
    float lenDir =
      GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f }, dirAxis, mvp, displayRatio);
    float lenDirMinus =
      GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f }, -dirAxis, mvp, displayRatio);

    auto mulAxis = (mAllowAxisFlip && lenDir < lenDirMinus &&
                    fabsf(lenDir - lenDirMinus) > FLT_EPSILON)
                     ? -1.f
                     : 1.f;
    dirAxis *= mulAxis;

    // axis is visible ?
    float axisLengthInClipSpace = GetSegmentLengthClipSpace(
      { 0.f, 0.f, 0.f }, dirAxis * screenFactor, mvp, displayRatio);
    belowAxisLimit = (axisLengthInClipSpace > 0.02f);
  }

  {
    // new method
    dirPlaneX = Vec4::DirectionUnary[(axisIndex + 1) % 3];
    dirPlaneY = Vec4::DirectionUnary[(axisIndex + 2) % 3];

    float lenDirPlaneX = GetSegmentLengthClipSpace(
      { 0.f, 0.f, 0.f }, dirPlaneX, mvp, displayRatio);
    float lenDirMinusPlaneX = GetSegmentLengthClipSpace(
      { 0.f, 0.f, 0.f }, -dirPlaneX, mvp, displayRatio);
    auto mulAxisX = (mAllowAxisFlip && lenDirPlaneX < lenDirMinusPlaneX &&
                     fabsf(lenDirPlaneX - lenDirMinusPlaneX) > FLT_EPSILON)
                      ? -1.f
                      : 1.f;
    dirPlaneX *= mulAxisX;

    float lenDirPlaneY = GetSegmentLengthClipSpace(
      { 0.f, 0.f, 0.f }, dirPlaneY, mvp, displayRatio);
    float lenDirMinusPlaneY = GetSegmentLengthClipSpace(
      { 0.f, 0.f, 0.f }, -dirPlaneY, mvp, displayRatio);
    auto mulAxisY = (mAllowAxisFlip && lenDirPlaneY < lenDirMinusPlaneY &&
                     fabsf(lenDirPlaneY - lenDirMinusPlaneY) > FLT_EPSILON)
                      ? -1.f
                      : 1.f;
    dirPlaneY *= mulAxisY;

    // plane is visible ?
    float paraSurf = GetParallelogram({ 0.f, 0.f, 0.f },
                                      dirPlaneX * screenFactor,
                                      dirPlaneY * screenFactor,
                                      mvp,
                                      displayRatio);
    belowPlaneLimit = (paraSurf > 0.0025f);
  }
}

}
