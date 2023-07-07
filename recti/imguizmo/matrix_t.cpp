#include "matrix_t.h"
#include <DirectXMath.h>
#include <cfloat>
#include <imgui.h>
#include <math.h>

static const float ZPI = 3.14159265358979323846f;
static const float RAD2DEG = (180.f / ZPI);
static const float DEG2RAD = (ZPI / 180.f);
static const vec_t directionUnary[3] = { { 1.f, 0.f, 0.f, 0 },
                                         { 0.f, 1.f, 0.f, 0 },
                                         { 0.f, 0.f, 1.f, 0 } };

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
matrix_t::Transpose()
{
  DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4*)this,
                           DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(
                             (const DirectX::XMFLOAT4X4*)this)));
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
    m00 = (srcMatrix.m11 * srcMatrix.m22 - srcMatrix.m12 * srcMatrix.m21) * s;
    m01 = (srcMatrix.m21 * srcMatrix.m02 - srcMatrix.m22 * srcMatrix.m01) * s;
    m02 = (srcMatrix.m01 * srcMatrix.m12 - srcMatrix.m02 * srcMatrix.m11) * s;
    m10 = (srcMatrix.m12 * srcMatrix.m20 - srcMatrix.m10 * srcMatrix.m22) * s;
    m11 = (srcMatrix.m22 * srcMatrix.m00 - srcMatrix.m20 * srcMatrix.m02) * s;
    m12 = (srcMatrix.m02 * srcMatrix.m10 - srcMatrix.m00 * srcMatrix.m12) * s;
    m20 = (srcMatrix.m10 * srcMatrix.m21 - srcMatrix.m11 * srcMatrix.m20) * s;
    m21 = (srcMatrix.m20 * srcMatrix.m01 - srcMatrix.m21 * srcMatrix.m00) * s;
    m22 = (srcMatrix.m00 * srcMatrix.m11 - srcMatrix.m01 * srcMatrix.m10) * s;
    m30 = -(m00 * srcMatrix.m30 + m10 * srcMatrix.m31 + m20 * srcMatrix.m32);
    m31 = -(m01 * srcMatrix.m30 + m11 * srcMatrix.m31 + m21 * srcMatrix.m32);
    m32 = -(m02 * srcMatrix.m30 + m12 * srcMatrix.m31 + m22 * srcMatrix.m32);
  } else {
    // transpose matrix
    float src[16];
    for (int i = 0; i < 4; ++i) {
      src[i] = srcMatrix[i * 4];
      src[i + 4] = srcMatrix[i * 4 + 1];
      src[i + 8] = srcMatrix[i * 4 + 2];
      src[i + 12] = srcMatrix[i * 4 + 3];
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
    (*this)[0] = (tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7]) -
                 (tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7]);
    (*this)[1] = (tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7]) -
                 (tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7]);
    (*this)[2] = (tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7]) -
                 (tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7]);
    (*this)[3] = (tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6]) -
                 (tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6]);
    (*this)[4] = (tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3]) -
                 (tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3]);
    (*this)[5] = (tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3]) -
                 (tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3]);
    (*this)[6] = (tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3]) -
                 (tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3]);
    (*this)[7] = (tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2]) -
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
    (*this)[8] = (tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15]) -
                 (tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15]);
    (*this)[9] = (tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15]) -
                 (tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15]);
    (*this)[10] = (tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15]) -
                  (tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15]);
    (*this)[11] = (tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14]) -
                  (tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14]);
    (*this)[12] = (tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9]) -
                  (tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10]);
    (*this)[13] = (tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10]) -
                  (tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8]);
    (*this)[14] = (tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8]) -
                  (tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9]);
    (*this)[15] = (tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9]) -
                  (tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8]);

    // calculate determinant
    det = src[0] * (*this)[0] + src[1] * (*this)[1] + src[2] * (*this)[2] +
          src[3] * (*this)[3];

    // calculate matrix inverse
    float invdet = 1 / det;
    for (int j = 0; j < 16; ++j) {
      (*this)[j] *= invdet;
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

  m00 = xx;
  m01 = xy + zs;
  m02 = zx - ys;
  m03 = 0.f;
  m10 = xy - zs;
  m11 = yy;
  m12 = yz + xs;
  m13 = 0.f;
  m20 = zx + ys;
  m21 = yz - xs;
  m22 = zz;
  m23 = 0.f;
  m30 = 0.f;
  m31 = 0.f;
  m32 = 0.f;
  m33 = 1.f;
}

void
Perspective(float fovyInDegrees,
            float aspectRatio,
            float znear,
            float zfar,
            float* m16)
{
  float ymax, xmax;
  ymax = znear * tanf(fovyInDegrees * DEG2RAD);
  xmax = ymax * aspectRatio;
  Frustum(-xmax, xmax, -ymax, ymax, znear, zfar, m16);
}

void
LookAt(const float* eye, const float* at, const float* up, float* m16)
{
  float X[3], Y[3], Z[3], tmp[3];

  tmp[0] = eye[0] - at[0];
  tmp[1] = eye[1] - at[1];
  tmp[2] = eye[2] - at[2];
  Normalize(tmp, Z);
  Normalize(up, Y);
  Cross(Y, Z, tmp);
  Normalize(tmp, X);
  Cross(Z, X, tmp);
  Normalize(tmp, Y);

  m16[0] = X[0];
  m16[1] = Y[0];
  m16[2] = Z[0];
  m16[3] = 0.0f;
  m16[4] = X[1];
  m16[5] = Y[1];
  m16[6] = Z[1];
  m16[7] = 0.0f;
  m16[8] = X[2];
  m16[9] = Y[2];
  m16[10] = Z[2];
  m16[11] = 0.0f;
  m16[12] = -Dot(X, eye);
  m16[13] = -Dot(Y, eye);
  m16[14] = -Dot(Z, eye);
  m16[15] = 1.0f;
}

ImVec2
worldToPos(const vec_t& worldPos, const matrix_t& mat, const vec_t& screenRect)
{
  vec_t trans;
  trans.TransformPoint(worldPos, mat);
  trans *= 0.5f / trans.w;
  trans += { 0.5f, 0.5f };
  trans.y = 1.f - trans.y;
  trans.x *= screenRect.z;
  trans.y *= screenRect.w;
  trans.x += screenRect.x;
  trans.y += screenRect.y;
  return ImVec2(trans.x, trans.y);
}

void
DecomposeMatrixToComponents(const float* matrix,
                            float* translation,
                            float* rotation,
                            float* scale)
{
  matrix_t mat = *(matrix_t*)matrix;

  scale[0] = mat.right().Length();
  scale[1] = mat.up().Length();
  scale[2] = mat.dir().Length();

  mat.OrthoNormalize();

  rotation[0] = RAD2DEG * atan2f(mat.m12, mat.m22);
  rotation[1] =
    RAD2DEG * atan2f(-mat.m02, sqrtf(mat.m12 * mat.m12 + mat.m22 * mat.m22));
  rotation[2] = RAD2DEG * atan2f(mat.m01, mat.m00);

  translation[0] = mat.position().x;
  translation[1] = mat.position().y;
  translation[2] = mat.position().z;
}

void
RecomposeMatrixFromComponents(const float* translation,
                              const float* rotation,
                              const float* scale,
                              float* matrix)
{
  matrix_t& mat = *(matrix_t*)matrix;

  matrix_t rot[3];
  for (int i = 0; i < 3; i++) {
    rot[i].RotationAxis(directionUnary[i], rotation[i] * DEG2RAD);
  }

  mat = rot[0] * rot[1] * rot[2];

  float validScale[3];
  for (int i = 0; i < 3; i++) {
    if (fabsf(scale[i]) < FLT_EPSILON) {
      validScale[i] = 0.001f;
    } else {
      validScale[i] = scale[i];
    }
  }
  mat.right() *= validScale[0];
  mat.up() *= validScale[1];
  mat.dir() *= validScale[2];
  mat.position() = { translation[0], translation[1], translation[2], 1.f };
}
