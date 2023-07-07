#include "vec_t.h"
#include "matrix_t.h"
#include <cfloat>
#include <imgui.h>
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
  return makeVect(fabsf(x), fabsf(y), fabsf(z));
}

vec_t
makeVect(const ImVec2& v)
{
  vec_t res;
  res.x = v.x;
  res.y = v.y;
  res.z = 0.f;
  res.w = 0.f;
  return res;
}

void
vec_t::Transform(const matrix_t& matrix)
{
  vec_t out;

  out.x = x * matrix.m[0][0] + y * matrix.m[1][0] + z * matrix.m[2][0] +
          w * matrix.m[3][0];
  out.y = x * matrix.m[0][1] + y * matrix.m[1][1] + z * matrix.m[2][1] +
          w * matrix.m[3][1];
  out.z = x * matrix.m[0][2] + y * matrix.m[1][2] + z * matrix.m[2][2] +
          w * matrix.m[3][2];
  out.w = x * matrix.m[0][3] + y * matrix.m[1][3] + z * matrix.m[2][3] +
          w * matrix.m[3][3];

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

  out.x = x * matrix.m[0][0] + y * matrix.m[1][0] + z * matrix.m[2][0] +
          matrix.m[3][0];
  out.y = x * matrix.m[0][1] + y * matrix.m[1][1] + z * matrix.m[2][1] +
          matrix.m[3][1];
  out.z = x * matrix.m[0][2] + y * matrix.m[1][2] + z * matrix.m[2][2] +
          matrix.m[3][2];
  out.w = x * matrix.m[0][3] + y * matrix.m[1][3] + z * matrix.m[2][3] +
          matrix.m[3][3];

  x = out.x;
  y = out.y;
  z = out.z;
  w = out.w;
}

void
vec_t::TransformVector(const matrix_t& matrix)
{
  vec_t out;

  out.x = x * matrix.m[0][0] + y * matrix.m[1][0] + z * matrix.m[2][0];
  out.y = x * matrix.m[0][1] + y * matrix.m[1][1] + z * matrix.m[2][1];
  out.z = x * matrix.m[0][2] + y * matrix.m[1][2] + z * matrix.m[2][2];
  out.w = x * matrix.m[0][3] + y * matrix.m[1][3] + z * matrix.m[2][3];

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
