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
