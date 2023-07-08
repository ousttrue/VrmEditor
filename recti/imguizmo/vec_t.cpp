#include "vec_t.h"
#include "matrix_t.h"
#include <cfloat>
// #include <imgui.h>
#include <math.h>
#include <memory.h>

float
vec_t::Length() const
{
  return sqrtf(x * x + y * y + z * z);
}

vec_t
vec_t::Normalize()
{
  (*this) *= (1.f / (Length() > FLT_EPSILON ? Length() : FLT_EPSILON));
  return (*this);
}

bool
vec_t::operator!=(const vec_t& other) const
{
  return memcmp(this, &other, sizeof(vec_t)) != 0;
}

vec_t
vec_t::Abs() const
{
  return { fabsf(x), fabsf(y), fabsf(z) };
}

void
vec_t::Transform(const matrix_t& matrix)
{
  vec_t out;
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
vec_t::Transform(const vec_t& s, const matrix_t& matrix)
{
  *this = s;
  Transform(matrix);
}

void
vec_t::TransformPoint(const matrix_t& matrix)
{
  vec_t out;
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
vec_t::TransformVector(const matrix_t& matrix)
{
  vec_t out;
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
IntersectRayPlane(const vec_t& rOrigin, const vec_t& rVector, const vec_t& plan)
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

vec_t
PointOnSegment(const vec_t& point, const vec_t& vertPos1, const vec_t& vertPos2)
{
  vec_t c = point - vertPos1;
  vec_t V;

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
ComputeSnap(vec_t& value, const float* snap)
{
  for (int i = 0; i < 3; i++) {
    ComputeSnap(&value[i], snap[i]);
  }
}

void
ComputeFrustumPlanes(vec_t* frustum, const float* clip)
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
