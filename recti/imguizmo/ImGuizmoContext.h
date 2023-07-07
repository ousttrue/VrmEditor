#pragma once
#include <imgui.h>

#include <functional>
#include <imgui_internal.h>

#ifdef USE_IMGUI_API
#include "imconfig.h"
#endif
#ifndef IMGUI_API
#define IMGUI_API
#endif

#ifndef IMGUIZMO_NAMESPACE
#define IMGUIZMO_NAMESPACE ImGuizmo
#endif

namespace IMGUIZMO_NAMESPACE {

enum MODE
{
  LOCAL,
  WORLD
};

// call it when you want a gizmo
// Needs view and projection matrices.
// matrix parameter is the source matrix (where will be gizmo be drawn) and
// might be transformed by the function. Return deltaMatrix is optional
// translation is applied in world space
enum OPERATION
{
  TRANSLATE_X = (1u << 0),
  TRANSLATE_Y = (1u << 1),
  TRANSLATE_Z = (1u << 2),
  ROTATE_X = (1u << 3),
  ROTATE_Y = (1u << 4),
  ROTATE_Z = (1u << 5),
  ROTATE_SCREEN = (1u << 6),
  SCALE_X = (1u << 7),
  SCALE_Y = (1u << 8),
  SCALE_Z = (1u << 9),
  BOUNDS = (1u << 10),
  SCALE_XU = (1u << 11),
  SCALE_YU = (1u << 12),
  SCALE_ZU = (1u << 13),

  TRANSLATE = TRANSLATE_X | TRANSLATE_Y | TRANSLATE_Z,
  ROTATE = ROTATE_X | ROTATE_Y | ROTATE_Z | ROTATE_SCREEN,
  SCALE = SCALE_X | SCALE_Y | SCALE_Z,
  SCALEU = SCALE_XU | SCALE_YU | SCALE_ZU, // universal
  UNIVERSAL = TRANSLATE | ROTATE | SCALEU
};

inline OPERATION
operator|(OPERATION lhs, OPERATION rhs)
{
  return static_cast<OPERATION>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline OPERATION&
operator|=(OPERATION& lhs, OPERATION rhs)
{
  lhs = lhs | rhs;
  return lhs;
}

enum COLOR
{
  DIRECTION_X,      // directionColor[0]
  DIRECTION_Y,      // directionColor[1]
  DIRECTION_Z,      // directionColor[2]
  PLANE_X,          // planeColor[0]
  PLANE_Y,          // planeColor[1]
  PLANE_Z,          // planeColor[2]
  SELECTION,        // selectionColor
  INACTIVE,         // inactiveColor
  TRANSLATION_LINE, // translationLineColor
  SCALE_LINE,
  ROTATION_USING_BORDER,
  ROTATION_USING_FILL,
  HATCHED_AXIS_LINES,
  TEXT,
  TEXT_SHADOW,
  COUNT
};

struct Style
{
  IMGUI_API Style();
  Style(const Style&) = delete;
  Style& operator=(const Style&) = delete;

  float TranslationLineThickness;   // Thickness of lines for translation gizmo
  float TranslationLineArrowSize;   // Size of arrow at the end of lines for
                                    // translation gizmo
  float RotationLineThickness;      // Thickness of lines for rotation gizmo
  float RotationOuterLineThickness; // Thickness of line surrounding the
                                    // rotation gizmo
  float ScaleLineThickness;         // Thickness of lines for scale gizmo
  float
    ScaleLineCircleSize; // Size of circle at the end of lines for scale gizmo
  float HatchedAxisLineThickness; // Thickness of hatched axis lines
  float CenterCircleSize; // Size of circle at the center of the translate/scale
                          // gizmo

  ImVec4 Colors[COLOR::COUNT];
};

struct matrix_t;
struct vec_t
{
public:
  float x, y, z, w;

  void Lerp(const vec_t& v, float t)
  {
    x += (v.x - x) * t;
    y += (v.y - y) * t;
    z += (v.z - z) * t;
    w += (v.w - w) * t;
  }

  void Set(float v) { x = y = z = w = v; }
  void Set(float _x, float _y, float _z = 0.f, float _w = 0.f)
  {
    x = _x;
    y = _y;
    z = _z;
    w = _w;
  }

  vec_t& operator-=(const vec_t& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
    return *this;
  }
  vec_t& operator+=(const vec_t& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
    return *this;
  }
  vec_t& operator*=(const vec_t& v)
  {
    x *= v.x;
    y *= v.y;
    z *= v.z;
    w *= v.w;
    return *this;
  }
  vec_t& operator*=(float v)
  {
    x *= v;
    y *= v;
    z *= v;
    w *= v;
    return *this;
  }

  vec_t operator*(float f) const;
  vec_t operator-() const;
  vec_t operator-(const vec_t& v) const;
  vec_t operator+(const vec_t& v) const;
  vec_t operator*(const vec_t& v) const;

  const vec_t& operator+() const { return (*this); }
  float Length() const { return sqrtf(x * x + y * y + z * z); };
  float LengthSq() const { return (x * x + y * y + z * z); };
  vec_t Normalize()
  {
    (*this) *= (1.f / (Length() > FLT_EPSILON ? Length() : FLT_EPSILON));
    return (*this);
  }
  vec_t Normalize(const vec_t& v)
  {
    this->Set(v.x, v.y, v.z, v.w);
    this->Normalize();
    return (*this);
  }
  vec_t Abs() const;

  void Cross(const vec_t& v)
  {
    vec_t res;
    res.x = y * v.z - z * v.y;
    res.y = z * v.x - x * v.z;
    res.z = x * v.y - y * v.x;

    x = res.x;
    y = res.y;
    z = res.z;
    w = 0.f;
  }

  void Cross(const vec_t& v1, const vec_t& v2)
  {
    x = v1.y * v2.z - v1.z * v2.y;
    y = v1.z * v2.x - v1.x * v2.z;
    z = v1.x * v2.y - v1.y * v2.x;
    w = 0.f;
  }

  float Dot(const vec_t& v) const
  {
    return (x * v.x) + (y * v.y) + (z * v.z) + (w * v.w);
  }

  float Dot3(const vec_t& v) const { return (x * v.x) + (y * v.y) + (z * v.z); }

  void Transform(const matrix_t& matrix);
  void Transform(const vec_t& s, const matrix_t& matrix);

  void TransformVector(const matrix_t& matrix);
  void TransformPoint(const matrix_t& matrix);
  void TransformVector(const vec_t& v, const matrix_t& matrix)
  {
    (*this) = v;
    this->TransformVector(matrix);
  }
  void TransformPoint(const vec_t& v, const matrix_t& matrix)
  {
    (*this) = v;
    this->TransformPoint(matrix);
  }

  float& operator[](size_t index) { return ((float*)&x)[index]; }
  const float& operator[](size_t index) const { return ((float*)&x)[index]; }
  bool operator!=(const vec_t& other) const
  {
    return memcmp(this, &other, sizeof(vec_t)) != 0;
  }
};

inline vec_t
makeVect(float _x, float _y, float _z = 0.f, float _w = 0.f)
{
  vec_t res;
  res.x = _x;
  res.y = _y;
  res.z = _z;
  res.w = _w;
  return res;
}
inline vec_t
makeVect(ImVec2 v)
{
  vec_t res;
  res.x = v.x;
  res.y = v.y;
  res.z = 0.f;
  res.w = 0.f;
  return res;
}
inline vec_t
vec_t::operator*(float f) const
{
  return makeVect(x * f, y * f, z * f, w * f);
}
inline vec_t
vec_t::operator-() const
{
  return makeVect(-x, -y, -z, -w);
}
inline vec_t
vec_t::operator-(const vec_t& v) const
{
  return makeVect(x - v.x, y - v.y, z - v.z, w - v.w);
}
inline vec_t
vec_t::operator+(const vec_t& v) const
{
  return makeVect(x + v.x, y + v.y, z + v.z, w + v.w);
}
inline vec_t
vec_t::operator*(const vec_t& v) const
{
  return makeVect(x * v.x, y * v.y, z * v.z, w * v.w);
}
inline vec_t
vec_t::Abs() const
{
  return makeVect(fabsf(x), fabsf(y), fabsf(z));
}

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

  void Multiply(const matrix_t& matrix)
  {
    matrix_t tmp;
    tmp = *this;

    FPU_MatrixF_x_MatrixF((float*)&tmp, (float*)&matrix, (float*)this);
  }

  void Multiply(const matrix_t& m1, const matrix_t& m2)
  {
    FPU_MatrixF_x_MatrixF((float*)&m1, (float*)&m2, (float*)this);
  }

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

struct Context
{
private:
  Context()
    : mbUsing(false)
    , mbEnable(true)
    , mbUsingBounds(false)
  {
  }

public:
  static Context& Instance()
  {
    static Context s_instance;
    return s_instance;
  }

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  ImDrawList* mDrawList;
  Style mStyle;

  MODE mMode;
  matrix_t mViewMat;
  matrix_t mProjectionMat;
  matrix_t mModel;
  matrix_t mModelLocal; // orthonormalized model
  matrix_t mModelInverse;
  matrix_t mModelSource;
  matrix_t mModelSourceInverse;
  matrix_t mMVP;
  matrix_t
    mMVPLocal; // MVP with full model matrix whereas mMVP's model matrix might
               // only be translation in case of World space edition
  matrix_t mViewProjection;

  vec_t mModelScaleOrigin;
  vec_t mCameraEye;
  vec_t mCameraRight;
  vec_t mCameraDir;
  vec_t mCameraUp;
  vec_t mRayOrigin;
  vec_t mRayVector;

  float mRadiusSquareCenter;
  ImVec2 mScreenSquareCenter;
  ImVec2 mScreenSquareMin;
  ImVec2 mScreenSquareMax;

  float mScreenFactor;
  vec_t mRelativeOrigin;

  bool mbUsing;
  bool mbEnable;
  bool mbMouseOver;
  bool mReversed; // reversed projection matrix

  // translation
  vec_t mTranslationPlan;
  vec_t mTranslationPlanOrigin;
  vec_t mMatrixOrigin;
  vec_t mTranslationLastDelta;

  // rotation
  vec_t mRotationVectorSource;
  float mRotationAngle;
  float mRotationAngleOrigin;
  // vec_t mWorldToLocalAxis;

  // scale
  vec_t mScale;
  vec_t mScaleValueOrigin;
  vec_t mScaleLast;
  float mSaveMousePosx;

  // save axis factor when using gizmo
  bool mBelowAxisLimit[3];
  bool mBelowPlaneLimit[3];
  float mAxisFactor[3];

  // bounds stretching
  vec_t mBoundsPivot;
  vec_t mBoundsAnchor;
  vec_t mBoundsPlan;
  vec_t mBoundsLocalPivot;
  int mBoundsBestAxis;
  int mBoundsAxis[2];
  bool mbUsingBounds;
  matrix_t mBoundsMatrix;

  //
  int mCurrentOperation;

  float mX = 0.f;
  float mY = 0.f;
  float mWidth = 0.f;
  float mHeight = 0.f;
  float mXMax = 0.f;
  float mYMax = 0.f;
  float mDisplayRatio = 1.f;

  bool mIsOrthographic = false;

  int mActualID = -1;
  int mEditingID = -1;
  OPERATION mOperation = OPERATION(-1);

  bool mAllowAxisFlip = true;
  float mGizmoSizeClipSpace = 0.1f;

  bool mAllowActiveHoverItem = false;

  void ComputeContext(const float* view,
                      const float* projection,
                      float* matrix,
                      MODE mode);
};

} // namespace IMGUIZMO_NAMESPACE
