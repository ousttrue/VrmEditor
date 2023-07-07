#pragma once
#include "vec_t.h"

struct matrix_t
{
  float m00, m01, m02, m03;
  float m10, m11, m12, m13;
  float m20, m21, m22, m23;
  float m30, m31, m32, m33;

  float& operator[](size_t index) { return (&m00)[index]; }
  float operator[](size_t index) const { return (&m00)[index]; }
  vec_t& right() { return *((vec_t*)&m00); }
  vec_t& up() { return *((vec_t*)&m10); }
  vec_t& dir() { return *((vec_t*)&m20); }
  vec_t& position() { return *((vec_t*)&m30); }
  const vec_t& right() const { return *((vec_t*)&m00); }
  const vec_t& up() const { return *((vec_t*)&m10); }
  const vec_t& dir() const { return *((vec_t*)&m20); }
  const vec_t& position() const { return *((vec_t*)&m30); }
  vec_t& component(size_t index) { return ((vec_t*)&m00)[index]; }
  const vec_t& component(size_t index) const { return ((vec_t*)&m00)[index]; }

  operator float*() { return &m11; }
  operator const float*() const { return &m11; }

  void Translation(const vec_t& vt)
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

  void Scale(const vec_t& s) { Scale(s.x, s.y, s.z); }

  matrix_t& operator*=(const matrix_t& mat)
  {
    matrix_t tmpMat;
    tmpMat = *this;
    tmpMat.Multiply(mat);
    *this = tmpMat;
    return *this;
  }

  matrix_t operator*(const matrix_t& mat) const
  {
    matrix_t matT;
    matT.Multiply(*this, mat);
    return matT;
  }

  void Multiply(const matrix_t& matrix);
  void Multiply(const matrix_t& m1, const matrix_t& m2);

  float GetDeterminant() const
  {
    return m00 * m11 * m22 + m01 * m12 * m20 + m02 * m10 * m21 -
           m02 * m11 * m20 - m01 * m10 * m22 - m00 * m12 * m21;
  }

  float Inverse(const matrix_t& srcMatrix, bool affine = false);
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

  void RotationAxis(const vec_t& axis, float angle);

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

ImVec2
worldToPos(const vec_t& worldPos, const matrix_t& mat, const vec_t& screenRect);

float
GetSegmentLengthClipSpace(const vec_t& start,
                          const vec_t& end,
                          const matrix_t& mvp,
                          float mDisplayRatio);

float
GetParallelogram(const vec_t& ptO,
                 const vec_t& ptA,
                 const vec_t& ptB,
                 const matrix_t& mMVP,
                 float mDisplayRatio);
