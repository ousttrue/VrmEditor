#include "tripod.h"
namespace recti {

inline bool
IsFlip(const Vec4& dirAxis, const Mat4& mvp, float aspectRatio)
{
  float lenDir =
    GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f }, dirAxis, mvp, aspectRatio);
  float lenDirMinus =
    GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f }, -dirAxis, mvp, aspectRatio);

  return (lenDir < lenDirMinus && fabsf(lenDir - lenDirMinus) > FLT_EPSILON);
}
// float lenDirPlaneX =
//   GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f }, dirPlaneX, mvp, aspectRatio);
// float lenDirMinusPlaneX = GetSegmentLengthClipSpace(
//   { 0.f, 0.f, 0.f }, -dirPlaneX, mvp, aspectRatio);
// auto mulAxisX = (allowFlipAxis && lenDirPlaneX < lenDirMinusPlaneX &&
//                  fabsf(lenDirPlaneX - lenDirMinusPlaneX) > FLT_EPSILON)
//                   ? -1.f
//                   : 1.f;
// dirPlaneX *= mulAxisX;

// float lenDirPlaneY =
//   GetSegmentLengthClipSpace({ 0.f, 0.f, 0.f }, dirPlaneY, mvp, aspectRatio);
// float lenDirMinusPlaneY = GetSegmentLengthClipSpace(
//   { 0.f, 0.f, 0.f }, -dirPlaneY, mvp, aspectRatio);
// auto mulAxisY = (allowFlipAxis && lenDirPlaneY < lenDirMinusPlaneY &&
//                  fabsf(lenDirPlaneY - lenDirMinusPlaneY) > FLT_EPSILON)
//                   ? -1.f
//                   : 1.f;
// dirPlaneY *= mulAxisY;

Tripod::Tripod(const Mat4& mvp,
               float aspectRatio,
               float screenFactor,
               bool allowFlipAxis,
               int axisIndex)
{
  {
    Axis = Vec4::DirectionUnary[axisIndex];
    if (allowFlipAxis && IsFlip(Axis, mvp, aspectRatio)) {
      Axis *= -1.0f;
    }

    // axis is visible ?
    float axisLengthInClipSpace = GetSegmentLengthClipSpace(
      { 0.f, 0.f, 0.f }, Axis * screenFactor, mvp, aspectRatio);
    VisibleAxis = (axisLengthInClipSpace > 0.02f);
  }

  {
    PlaneX = Vec4::DirectionUnary[(axisIndex + 1) % 3];
    if (allowFlipAxis && IsFlip(PlaneX, mvp, aspectRatio)) {
      PlaneX *= -1.0f;
    }

    PlaneY = Vec4::DirectionUnary[(axisIndex + 2) % 3];
    if (allowFlipAxis && IsFlip(PlaneY, mvp, aspectRatio)) {
      PlaneY *= -1.0f;
    }

    // plane is visible ?
    float paraSurf = GetParallelogram({ 0.f, 0.f, 0.f },
                                      PlaneX * screenFactor,
                                      PlaneY * screenFactor,
                                      mvp,
                                      aspectRatio);
    VisiblePlane = (paraSurf > 0.0025f);
  }
}

}
