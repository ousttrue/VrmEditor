#pragma once
#include "vec4.h"
#include <tuple>

namespace recti {

struct Mat4
{
  float m00, m01, m02, m03;
  float m10, m11, m12, m13;
  float m20, m21, m22, m23;
  float m30, m31, m32, m33;

  float& operator[](size_t index) { return (&m00)[index]; }
  float operator[](size_t index) const { return (&m00)[index]; }
  Vec4& right() { return *((Vec4*)&m00); }
  Vec4& up() { return *((Vec4*)&m10); }
  Vec4& dir() { return *((Vec4*)&m20); }
  Vec4& position() { return *((Vec4*)&m30); }
  const Vec4& right() const { return *((Vec4*)&m00); }
  const Vec4& up() const { return *((Vec4*)&m10); }
  const Vec4& dir() const { return *((Vec4*)&m20); }
  const Vec4& position() const { return *((Vec4*)&m30); }
  Vec4& component(size_t index) { return ((Vec4*)&m00)[index]; }
  const Vec4& component(size_t index) const { return ((Vec4*)&m00)[index]; }

  operator float*() { return &m11; }
  operator const float*() const { return &m11; }

  void Translation(const Vec4& vt)
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

  void Scale(const Vec4& s) { Scale(s.x, s.y, s.z); }

  Mat4& operator*=(const Mat4& mat)
  {
    Mat4 tmpMat;
    tmpMat = *this;
    tmpMat.Multiply(mat);
    *this = tmpMat;
    return *this;
  }

  Mat4 operator*(const Mat4& mat) const
  {
    Mat4 matT;
    matT.Multiply(*this, mat);
    return matT;
  }

  void Multiply(const Mat4& matrix);
  void Multiply(const Mat4& m1, const Mat4& m2);

  float GetDeterminant() const
  {
    return m00 * m11 * m22 + m01 * m12 * m20 + m02 * m10 * m21 -
           m02 * m11 * m20 - m01 * m10 * m22 - m00 * m12 * m21;
  }

  float Inverse(const Mat4& srcMatrix, bool affine = false);
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

  void RotationAxis(const Vec4& axis, float angle);

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
worldToPos(const Vec4& worldPos, const Mat4& mat, const Vec4& screenRect);

float
GetSegmentLengthClipSpace(const Vec4& start,
                          const Vec4& end,
                          const Mat4& mvp,
                          float mDisplayRatio);

float
GetParallelogram(const Vec4& ptO,
                 const Vec4& ptA,
                 const Vec4& ptB,
                 const Mat4& mMVP,
                 float mDisplayRatio);

}
