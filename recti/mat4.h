#pragma once
#include "vec4.h"
#include <tuple>

namespace recti {

struct mat4
{
  float m00, m01, m02, m03;
  float m10, m11, m12, m13;
  float m20, m21, m22, m23;
  float m30, m31, m32, m33;

  float& operator[](size_t index) { return (&m00)[index]; }
  float operator[](size_t index) const { return (&m00)[index]; }
  vec4& right() { return *((vec4*)&m00); }
  vec4& up() { return *((vec4*)&m10); }
  vec4& dir() { return *((vec4*)&m20); }
  vec4& position() { return *((vec4*)&m30); }
  const vec4& right() const { return *((vec4*)&m00); }
  const vec4& up() const { return *((vec4*)&m10); }
  const vec4& dir() const { return *((vec4*)&m20); }
  const vec4& position() const { return *((vec4*)&m30); }
  vec4& component(size_t index) { return ((vec4*)&m00)[index]; }
  const vec4& component(size_t index) const { return ((vec4*)&m00)[index]; }

  operator float*() { return &m11; }
  operator const float*() const { return &m11; }

  void Translation(const vec4& vt)
  {
    *this = { 1.f,  0.f,  0.f,  0.f, //
              0.f,  1.f,  0.f,  0.f, //
              0.f,  0.f,  1.f,  0.f, //
              vt.x, vt.y, vt.z, 1.f };
  }

  void Scale(float _x, float _y, float _z)
  {
    *this = {
      _x,  0.f, 0.f, 0.f, //
      0.f, _y,  0.f, 0.f, //
      0.f, 0.f, _z,  0.f, //
      0.f, 0.f, 0.f, 1.f,
    };
  }

  void Scale(const vec4& s) { Scale(s.x, s.y, s.z); }

  mat4& operator*=(const mat4& mat)
  {
    mat4 tmpMat;
    tmpMat = *this;
    tmpMat.Multiply(mat);
    *this = tmpMat;
    return *this;
  }

  mat4 operator*(const mat4& mat) const
  {
    mat4 matT;
    matT.Multiply(*this, mat);
    return matT;
  }

  void Multiply(const mat4& matrix);
  void Multiply(const mat4& m1, const mat4& m2);

  float GetDeterminant() const
  {
    return m00 * m11 * m22 + m01 * m12 * m20 + m02 * m10 * m21 -
           m02 * m11 * m20 - m01 * m10 * m22 - m00 * m12 * m21;
  }

  float Inverse(const mat4& srcMatrix, bool affine = false);
  void SetToIdentity()
  {
    *this = {
      1.f, 0.f, 0.f, 0.f, //
      0.f, 1.f, 0.f, 0.f, //
      0.f, 0.f, 1.f, 0.f, //
      0.f, 0.f, 0.f, 1.f,
    };
  }
  void Transpose();

  void RotationAxis(const vec4& axis, float angle);

  void OrthoNormalize()
  {
    right().Normalize();
    up().Normalize();
    dir().Normalize();
  }
};

inline void
Frustum(float left,
        float right,
        float bottom,
        float top,
        float znear,
        float zfar,
        float* m16)
{
  float temp, temp2, temp3, temp4;
  temp = 2.0f * znear;
  temp2 = right - left;
  temp3 = top - bottom;
  temp4 = zfar - znear;
  m16[0] = temp / temp2;
  m16[1] = 0.0;
  m16[2] = 0.0;
  m16[3] = 0.0;
  m16[4] = 0.0;
  m16[5] = temp / temp3;
  m16[6] = 0.0;
  m16[7] = 0.0;
  m16[8] = (right + left) / temp2;
  m16[9] = (top + bottom) / temp3;
  m16[10] = (-zfar - znear) / temp4;
  m16[11] = -1.0f;
  m16[12] = 0.0;
  m16[13] = 0.0;
  m16[14] = (-temp * zfar) / temp4;
  m16[15] = 0.0;
}

void
Perspective(float fovyInDegrees,
            float aspectRatio,
            float znear,
            float zfar,
            float* m16);

void
LookAt(const float* eye, const float* at, const float* up, float* m16);

std::tuple<float, float>
worldToPos(const vec4& worldPos, const mat4& mat, const vec4& screenRect);

float
GetSegmentLengthClipSpace(const vec4& start,
                          const vec4& end,
                          const mat4& mvp,
                          float mDisplayRatio);

float
GetParallelogram(const vec4& ptO,
                 const vec4& ptA,
                 const vec4& ptB,
                 const mat4& mMVP,
                 float mDisplayRatio);

}
