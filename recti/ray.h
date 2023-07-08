#pragma once
#include "camera_mouse.h"
#include <float.h>
#include <math.h>

namespace recti {

struct Ray
{
  Vec4 mRayOrigin;
  Vec4 mRayVector;

  void Initialize(const Camera& camera, const Vec2& mousePos, bool mReversed)
  {
    Mat4 mViewProjInverse;
    mViewProjInverse.Inverse(camera.ViewMatrix * camera.ProjectionMatrix);

    const float mox =
      ((mousePos.X - camera.Viewport.x) / camera.Viewport.z) * 2.f - 1.f;
    const float moy =
      (1.f - ((mousePos.Y - camera.Viewport.y) / camera.Viewport.w)) * 2.f -
      1.f;

    const float zNear = mReversed ? (1.f - FLT_EPSILON) : 0.f;
    const float zFar = mReversed ? 0.f : (1.f - FLT_EPSILON);

    mRayOrigin.Transform({ mox, moy, zNear, 1.f }, mViewProjInverse);
    mRayOrigin *= 1.f / mRayOrigin.w;
    Vec4 rayEnd;
    rayEnd.Transform({ mox, moy, zFar, 1.f }, mViewProjInverse);
    rayEnd *= 1.f / rayEnd.w;
    mRayVector = Normalized(rayEnd - mRayOrigin);
  }

  float IntersectPlaneDistance(const Vec4& plane) const
  {
    const float numer = plane.Dot3(mRayOrigin) - plane.w;
    const float denom = plane.Dot3(mRayVector);

    if (fabsf(denom) <
        FLT_EPSILON) // normal is orthogonal to vector, cant intersect
    {
      return -1.0f;
    }

    return -(numer / denom);
  }

  Vec4 IntersectPlane(const Vec4& plane) const
  {
    const float signedLength = IntersectPlaneDistance(plane);
    // ?
    const float len = fabsf(signedLength); // near plan
    return mRayOrigin + mRayVector * len;
  }
};

} // namespace
