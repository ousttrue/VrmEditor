#pragma once
#include "vec_t.h"

struct matrix_t
{
public:
  union
  {
    float m[4][4];
    float m16[16];
    struct
    {
      vec_t right, up, dir, position;
    } v;
    vec_t component[4];
  };

  operator float*() { return m16; }
  operator const float*() const { return m16; }
  void Translation(float _x, float _y, float _z)
  {
    this->Translation(makeVect(_x, _y, _z));
  }

  void Translation(const vec_t& vt)
  {
    v.right.Set(1.f, 0.f, 0.f, 0.f);
    v.up.Set(0.f, 1.f, 0.f, 0.f);
    v.dir.Set(0.f, 0.f, 1.f, 0.f);
    v.position.Set(vt.x, vt.y, vt.z, 1.f);
  }

  void Scale(float _x, float _y, float _z)
  {
    v.right.Set(_x, 0.f, 0.f, 0.f);
    v.up.Set(0.f, _y, 0.f, 0.f);
    v.dir.Set(0.f, 0.f, _z, 0.f);
    v.position.Set(0.f, 0.f, 0.f, 1.f);
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
    return m[0][0] * m[1][1] * m[2][2] + m[0][1] * m[1][2] * m[2][0] +
           m[0][2] * m[1][0] * m[2][1] - m[0][2] * m[1][1] * m[2][0] -
           m[0][1] * m[1][0] * m[2][2] - m[0][0] * m[1][2] * m[2][1];
  }

  float Inverse(const matrix_t& srcMatrix, bool affine = false);
  void SetToIdentity()
  {
    v.right.Set(1.f, 0.f, 0.f, 0.f);
    v.up.Set(0.f, 1.f, 0.f, 0.f);
    v.dir.Set(0.f, 0.f, 1.f, 0.f);
    v.position.Set(0.f, 0.f, 0.f, 1.f);
  }
  void Transpose()
  {
    matrix_t tmpm;
    for (int l = 0; l < 4; l++) {
      for (int c = 0; c < 4; c++) {
        tmpm.m[l][c] = m[c][l];
      }
    }
    (*this) = tmpm;
  }

  void RotationAxis(const vec_t& axis, float angle);

  void OrthoNormalize()
  {
    v.right.Normalize();
    v.up.Normalize();
    v.dir.Normalize();
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
worldToPos(const vec_t& worldPos,
           const matrix_t& mat,
           const vec_t& screenRect);
