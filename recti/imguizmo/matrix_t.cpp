#include "matrix_t.h"
#include <cfloat>
#include <math.h>

inline void
FPU_MatrixF_x_MatrixF(const float* a, const float* b, float* r)
{
  r[0] = a[0] * b[0] + a[1] * b[4] + a[2] * b[8] + a[3] * b[12];
  r[1] = a[0] * b[1] + a[1] * b[5] + a[2] * b[9] + a[3] * b[13];
  r[2] = a[0] * b[2] + a[1] * b[6] + a[2] * b[10] + a[3] * b[14];
  r[3] = a[0] * b[3] + a[1] * b[7] + a[2] * b[11] + a[3] * b[15];

  r[4] = a[4] * b[0] + a[5] * b[4] + a[6] * b[8] + a[7] * b[12];
  r[5] = a[4] * b[1] + a[5] * b[5] + a[6] * b[9] + a[7] * b[13];
  r[6] = a[4] * b[2] + a[5] * b[6] + a[6] * b[10] + a[7] * b[14];
  r[7] = a[4] * b[3] + a[5] * b[7] + a[6] * b[11] + a[7] * b[15];

  r[8] = a[8] * b[0] + a[9] * b[4] + a[10] * b[8] + a[11] * b[12];
  r[9] = a[8] * b[1] + a[9] * b[5] + a[10] * b[9] + a[11] * b[13];
  r[10] = a[8] * b[2] + a[9] * b[6] + a[10] * b[10] + a[11] * b[14];
  r[11] = a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11] * b[15];

  r[12] = a[12] * b[0] + a[13] * b[4] + a[14] * b[8] + a[15] * b[12];
  r[13] = a[12] * b[1] + a[13] * b[5] + a[14] * b[9] + a[15] * b[13];
  r[14] = a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14];
  r[15] = a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15];
}

void
matrix_t::Multiply(const matrix_t& matrix)
{
  matrix_t tmp;
  tmp = *this;

  FPU_MatrixF_x_MatrixF((float*)&tmp, (float*)&matrix, (float*)this);
}

void
matrix_t::Multiply(const matrix_t& m1, const matrix_t& m2)
{
  FPU_MatrixF_x_MatrixF((float*)&m1, (float*)&m2, (float*)this);
}

float
matrix_t::Inverse(const matrix_t& srcMatrix, bool affine)
{
  float det = 0;

  if (affine) {
    det = GetDeterminant();
    float s = 1 / det;
    m[0][0] = (srcMatrix.m[1][1] * srcMatrix.m[2][2] -
               srcMatrix.m[1][2] * srcMatrix.m[2][1]) *
              s;
    m[0][1] = (srcMatrix.m[2][1] * srcMatrix.m[0][2] -
               srcMatrix.m[2][2] * srcMatrix.m[0][1]) *
              s;
    m[0][2] = (srcMatrix.m[0][1] * srcMatrix.m[1][2] -
               srcMatrix.m[0][2] * srcMatrix.m[1][1]) *
              s;
    m[1][0] = (srcMatrix.m[1][2] * srcMatrix.m[2][0] -
               srcMatrix.m[1][0] * srcMatrix.m[2][2]) *
              s;
    m[1][1] = (srcMatrix.m[2][2] * srcMatrix.m[0][0] -
               srcMatrix.m[2][0] * srcMatrix.m[0][2]) *
              s;
    m[1][2] = (srcMatrix.m[0][2] * srcMatrix.m[1][0] -
               srcMatrix.m[0][0] * srcMatrix.m[1][2]) *
              s;
    m[2][0] = (srcMatrix.m[1][0] * srcMatrix.m[2][1] -
               srcMatrix.m[1][1] * srcMatrix.m[2][0]) *
              s;
    m[2][1] = (srcMatrix.m[2][0] * srcMatrix.m[0][1] -
               srcMatrix.m[2][1] * srcMatrix.m[0][0]) *
              s;
    m[2][2] = (srcMatrix.m[0][0] * srcMatrix.m[1][1] -
               srcMatrix.m[0][1] * srcMatrix.m[1][0]) *
              s;
    m[3][0] = -(m[0][0] * srcMatrix.m[3][0] + m[1][0] * srcMatrix.m[3][1] +
                m[2][0] * srcMatrix.m[3][2]);
    m[3][1] = -(m[0][1] * srcMatrix.m[3][0] + m[1][1] * srcMatrix.m[3][1] +
                m[2][1] * srcMatrix.m[3][2]);
    m[3][2] = -(m[0][2] * srcMatrix.m[3][0] + m[1][2] * srcMatrix.m[3][1] +
                m[2][2] * srcMatrix.m[3][2]);
  } else {
    // transpose matrix
    float src[16];
    for (int i = 0; i < 4; ++i) {
      src[i] = srcMatrix.m16[i * 4];
      src[i + 4] = srcMatrix.m16[i * 4 + 1];
      src[i + 8] = srcMatrix.m16[i * 4 + 2];
      src[i + 12] = srcMatrix.m16[i * 4 + 3];
    }

    // calculate pairs for first 8 elements (cofactors)
    float tmp[12]; // temp array for pairs
    tmp[0] = src[10] * src[15];
    tmp[1] = src[11] * src[14];
    tmp[2] = src[9] * src[15];
    tmp[3] = src[11] * src[13];
    tmp[4] = src[9] * src[14];
    tmp[5] = src[10] * src[13];
    tmp[6] = src[8] * src[15];
    tmp[7] = src[11] * src[12];
    tmp[8] = src[8] * src[14];
    tmp[9] = src[10] * src[12];
    tmp[10] = src[8] * src[13];
    tmp[11] = src[9] * src[12];

    // calculate first 8 elements (cofactors)
    m16[0] = (tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7]) -
             (tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7]);
    m16[1] = (tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7]) -
             (tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7]);
    m16[2] = (tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7]) -
             (tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7]);
    m16[3] = (tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6]) -
             (tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6]);
    m16[4] = (tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3]) -
             (tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3]);
    m16[5] = (tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3]) -
             (tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3]);
    m16[6] = (tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3]) -
             (tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3]);
    m16[7] = (tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2]) -
             (tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2]);

    // calculate pairs for second 8 elements (cofactors)
    tmp[0] = src[2] * src[7];
    tmp[1] = src[3] * src[6];
    tmp[2] = src[1] * src[7];
    tmp[3] = src[3] * src[5];
    tmp[4] = src[1] * src[6];
    tmp[5] = src[2] * src[5];
    tmp[6] = src[0] * src[7];
    tmp[7] = src[3] * src[4];
    tmp[8] = src[0] * src[6];
    tmp[9] = src[2] * src[4];
    tmp[10] = src[0] * src[5];
    tmp[11] = src[1] * src[4];

    // calculate second 8 elements (cofactors)
    m16[8] = (tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15]) -
             (tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15]);
    m16[9] = (tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15]) -
             (tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15]);
    m16[10] = (tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15]) -
              (tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15]);
    m16[11] = (tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14]) -
              (tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14]);
    m16[12] = (tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9]) -
              (tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10]);
    m16[13] = (tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10]) -
              (tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8]);
    m16[14] = (tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8]) -
              (tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9]);
    m16[15] = (tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9]) -
              (tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8]);

    // calculate determinant
    det = src[0] * m16[0] + src[1] * m16[1] + src[2] * m16[2] + src[3] * m16[3];

    // calculate matrix inverse
    float invdet = 1 / det;
    for (int j = 0; j < 16; ++j) {
      m16[j] *= invdet;
    }
  }

  return det;
}

void
matrix_t::RotationAxis(const vec_t& axis, float angle)
{
  float length2 = axis.LengthSq();
  if (length2 < FLT_EPSILON) {
    SetToIdentity();
    return;
  }

  vec_t n = axis * (1.f / sqrtf(length2));
  float s = sinf(angle);
  float c = cosf(angle);
  float k = 1.f - c;

  float xx = n.x * n.x * k + c;
  float yy = n.y * n.y * k + c;
  float zz = n.z * n.z * k + c;
  float xy = n.x * n.y * k;
  float yz = n.y * n.z * k;
  float zx = n.z * n.x * k;
  float xs = n.x * s;
  float ys = n.y * s;
  float zs = n.z * s;

  m[0][0] = xx;
  m[0][1] = xy + zs;
  m[0][2] = zx - ys;
  m[0][3] = 0.f;
  m[1][0] = xy - zs;
  m[1][1] = yy;
  m[1][2] = yz + xs;
  m[1][3] = 0.f;
  m[2][0] = zx + ys;
  m[2][1] = yz - xs;
  m[2][2] = zz;
  m[2][3] = 0.f;
  m[3][0] = 0.f;
  m[3][1] = 0.f;
  m[3][2] = 0.f;
  m[3][3] = 1.f;
}
