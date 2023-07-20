#pragma once
#include "mat4.h"
#include "vec2.h"
#include <float.h>
#include <math.h>

namespace recti {

struct Ray
{
  Vec4 mRayOrigin;
  Vec4 mRayVector;

  void Initialize(const Mat4& view,
                  const Mat4& projection,
                  const Vec4& viewport,
                  const Vec2& mousePos)
  {
    Mat4 mViewProjInverse;
    mViewProjInverse.Inverse(view * projection);

    const float mox = ((mousePos.x - viewport.x) / viewport.z) * 2.f - 1.f;
    const float moy =
      (1.f - ((mousePos.y - viewport.y) / viewport.w)) * 2.f - 1.f;

    // projection reverse(OpenGL or DirectX) ?
    recti::Vec4 nearPos, farPos;
    nearPos.Transform({ 0, 0, 1.f, 1.f }, projection);
    bool mReversed = (nearPos.z / nearPos.w) > (farPos.z / farPos.w);

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
