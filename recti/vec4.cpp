#include "Mat4.h"
#include "vec4.h"
#include <cfloat>
#include <math.h>
#include <memory.h>

namespace recti {

float
Vec4::Length() const
{
  return sqrtf(x * x + y * y + z * z);
}

Vec4
Vec4::Normalize()
{
  (*this) *= (1.f / (Length() > FLT_EPSILON ? Length() : FLT_EPSILON));
  return (*this);
}

bool
Vec4::operator!=(const Vec4& other) const
{
  return memcmp(this, &other, sizeof(Vec4)) != 0;
}

Vec4
Vec4::Abs() const
{
  return { fabsf(x), fabsf(y), fabsf(z) };
}

void
Vec4::Transform(const Mat4& matrix)
{
  Vec4 out;
  out.x = x * matrix.m00 + y * matrix.m10 + z * matrix.m20 + w * matrix.m30;
  out.y = x * matrix.m01 + y * matrix.m11 + z * matrix.m21 + w * matrix.m31;
  out.z = x * matrix.m02 + y * matrix.m12 + z * matrix.m22 + w * matrix.m32;
  out.w = x * matrix.m03 + y * matrix.m13 + z * matrix.m23 + w * matrix.m33;
  x = out.x;
  y = out.y;
  z = out.z;
  w = out.w;
}

void
Vec4::Transform(const Vec4& s, const Mat4& matrix)
{
  *this = s;
  Transform(matrix);
}

void
Vec4::TransformPoint(const Mat4& matrix)
{
  Vec4 out;
  out.x = x * matrix.m00 + y * matrix.m10 + z * matrix.m20 + matrix.m30;
  out.y = x * matrix.m01 + y * matrix.m11 + z * matrix.m21 + matrix.m31;
  out.z = x * matrix.m02 + y * matrix.m12 + z * matrix.m22 + matrix.m32;
  out.w = x * matrix.m03 + y * matrix.m13 + z * matrix.m23 + matrix.m33;
  x = out.x;
  y = out.y;
  z = out.z;
  w = out.w;
}

void
Vec4::TransformVector(const Mat4& matrix)
{
  Vec4 out;
  out.x = x * matrix.m00 + y * matrix.m10 + z * matrix.m20;
  out.y = x * matrix.m01 + y * matrix.m11 + z * matrix.m21;
  out.z = x * matrix.m02 + y * matrix.m12 + z * matrix.m22;
  out.w = x * matrix.m03 + y * matrix.m13 + z * matrix.m23;
  x = out.x;
  y = out.y;
  z = out.z;
  w = out.w;
}

void
Normalize(const float* a, float* r)
{
  float il = 1.f / (sqrtf(Dot(a, a)) + FLT_EPSILON);
  r[0] = a[0] * il;
  r[1] = a[1] * il;
  r[2] = a[2] * il;
}

float
IntersectRayPlane(const Vec4& rOrigin, const Vec4& rVector, const Vec4& plan)
{
  const float numer = plan.Dot3(rOrigin) - plan.w;
  const float denom = plan.Dot3(rVector);

  if (fabsf(denom) <
      FLT_EPSILON) // normal is orthogonal to vector, cant intersect
  {
    return -1.0f;
  }

  return -(numer / denom);
}

Vec4
PointOnSegment(const Vec4& point, const Vec4& vertPos1, const Vec4& vertPos2)
{
  Vec4 c = point - vertPos1;
  Vec4 V;

  V.Normalize(vertPos2 - vertPos1);
  float d = (vertPos2 - vertPos1).Length();
  float t = V.Dot3(c);

  if (t < 0.f) {
    return vertPos1;
  }

  if (t > d) {
    return vertPos2;
  }

  return vertPos1 + V * t;
}

static const float snapTension = 0.5f;
void
ComputeSnap(float* value, float snap)
{
  if (snap <= FLT_EPSILON) {
    return;
  }

  float modulo = fmodf(*value, snap);
  float moduloRatio = fabsf(modulo) / snap;
  if (moduloRatio < snapTension) {
    *value -= modulo;
  } else if (moduloRatio > (1.f - snapTension)) {
    *value = *value - modulo + snap * ((*value < 0.f) ? -1.f : 1.f);
  }
}
void
ComputeSnap(Vec4& value, const float* snap)
{
  for (int i = 0; i < 3; i++) {
    ComputeSnap(&value[i], snap[i]);
  }
}

void
ComputeFrustumPlanes(Vec4* frustum, const float* clip)
{
  frustum[0].x = clip[3] - clip[0];
  frustum[0].y = clip[7] - clip[4];
  frustum[0].z = clip[11] - clip[8];
  frustum[0].w = clip[15] - clip[12];

  frustum[1].x = clip[3] + clip[0];
  frustum[1].y = clip[7] + clip[4];
  frustum[1].z = clip[11] + clip[8];
  frustum[1].w = clip[15] + clip[12];

  frustum[2].x = clip[3] + clip[1];
  frustum[2].y = clip[7] + clip[5];
  frustum[2].z = clip[11] + clip[9];
  frustum[2].w = clip[15] + clip[13];

  frustum[3].x = clip[3] - clip[1];
  frustum[3].y = clip[7] - clip[5];
  frustum[3].z = clip[11] - clip[9];
  frustum[3].w = clip[15] - clip[13];

  frustum[4].x = clip[3] - clip[2];
  frustum[4].y = clip[7] - clip[6];
  frustum[4].z = clip[11] - clip[10];
  frustum[4].w = clip[15] - clip[14];

  frustum[5].x = clip[3] + clip[2];
  frustum[5].y = clip[7] + clip[6];
  frustum[5].z = clip[11] + clip[10];
  frustum[5].w = clip[15] + clip[14];

  for (int i = 0; i < 6; i++) {
    frustum[i].Normalize();
  }
}

} // namespace
