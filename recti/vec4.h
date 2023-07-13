#pragma once

namespace recti {

struct Mat4;
struct Vec4
{
  float x = 0;
  float y = 0;
  float z = 0;
  float w = 0;

  void Lerp(const Vec4& v, float t)
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

  Vec4& operator-=(const Vec4& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
    return *this;
  }
  Vec4& operator+=(const Vec4& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
    return *this;
  }
  Vec4& operator*=(const Vec4& v)
  {
    x *= v.x;
    y *= v.y;
    z *= v.z;
    w *= v.w;
    return *this;
  }
  Vec4& operator*=(float v)
  {
    x *= v;
    y *= v;
    z *= v;
    w *= v;
    return *this;
  }

  Vec4 operator*(float f) const;
  Vec4 operator-() const;
  Vec4 operator-(const Vec4& v) const;
  Vec4 operator+(const Vec4& v) const;
  Vec4 operator*(const Vec4& v) const;

  const Vec4& operator+() const { return (*this); }
  float Length() const;
  float LengthSq() const { return (x * x + y * y + z * z); };
  Vec4 Normalize();
  Vec4 Normalize(const Vec4& v)
  {
    this->Set(v.x, v.y, v.z, v.w);
    this->Normalize();
    return (*this);
  }
  Vec4 Abs() const;

  void Cross(const Vec4& v)
  {
    Vec4 res;
    res.x = y * v.z - z * v.y;
    res.y = z * v.x - x * v.z;
    res.z = x * v.y - y * v.x;

    x = res.x;
    y = res.y;
    z = res.z;
    w = 0.f;
  }

  void Cross(const Vec4& v1, const Vec4& v2)
  {
    x = v1.y * v2.z - v1.z * v2.y;
    y = v1.z * v2.x - v1.x * v2.z;
    z = v1.x * v2.y - v1.y * v2.x;
    w = 0.f;
  }

  float Dot(const Vec4& v) const
  {
    return (x * v.x) + (y * v.y) + (z * v.z) + (w * v.w);
  }

  float Dot3(const Vec4& v) const { return (x * v.x) + (y * v.y) + (z * v.z); }

  void Transform(const Mat4& matrix);
  void Transform(const Vec4& s, const Mat4& matrix);

  void TransformVector(const Mat4& matrix);
  void TransformPoint(const Mat4& matrix);
  void TransformVector(const Vec4& v, const Mat4& matrix)
  {
    (*this) = v;
    this->TransformVector(matrix);
  }
  void TransformPoint(const Vec4& v, const Mat4& matrix)
  {
    (*this) = v;
    this->TransformPoint(matrix);
  }

  float& operator[](size_t index) { return ((float*)&x)[index]; }
  const float& operator[](size_t index) const { return ((float*)&x)[index]; }
  bool operator!=(const Vec4& other) const;

  static const Vec4 DirectionUnary[3];
};

inline const Vec4 Vec4::DirectionUnary[3] = { { 1.f, 0.f, 0.f, 0 },
                                              { 0.f, 1.f, 0.f, 0 },
                                              { 0.f, 0.f, 1.f, 0 } };

inline Vec4
Vec4::operator*(float f) const
{
  return { x * f, y * f, z * f, w * f };
}

inline Vec4
Vec4::operator-() const
{
  return { -x, -y, -z, -w };
}

inline Vec4
Vec4::operator-(const Vec4& v) const
{
  return { x - v.x, y - v.y, z - v.z, w - v.w };
}

inline Vec4
Vec4::operator+(const Vec4& v) const
{
  return { x + v.x, y + v.y, z + v.z, w + v.w };
}

inline Vec4
Vec4::operator*(const Vec4& v) const
{
  return { x * v.x, y * v.y, z * v.z, w * v.w };
}

// utility and math

inline void
Cross(const float* a, const float* b, float* r)
{
  r[0] = a[1] * b[2] - a[2] * b[1];
  r[1] = a[2] * b[0] - a[0] * b[2];
  r[2] = a[0] * b[1] - a[1] * b[0];
}

inline float
Dot(const float* a, const float* b)
{
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void
Normalize(const float* a, float* r);

template<typename T>
T
Clamp(T x, T y, T z)
{
  return ((x < y) ? y : ((x > z) ? z : x));
}
template<typename T>
T
max(T x, T y)
{
  return (x > y) ? x : y;
}
template<typename T>
T
min(T x, T y)
{
  return (x < y) ? x : y;
}
template<typename T>
bool
IsWithin(T x, T y, T z)
{
  return (x >= y) && (x <= z);
}

inline Vec4
Normalized(const Vec4& v)
{
  Vec4 res;
  res = v;
  res.Normalize();
  return res;
}

inline Vec4
Cross(const Vec4& v1, const Vec4& v2)
{
  Vec4 res;
  res.x = v1.y * v2.z - v1.z * v2.y;
  res.y = v1.z * v2.x - v1.x * v2.z;
  res.z = v1.x * v2.y - v1.y * v2.x;
  res.w = 0.f;
  return res;
}

inline float
Dot(const Vec4& v1, const Vec4& v2)
{
  return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

inline Vec4
BuildPlan(const Vec4& p_point1, const Vec4& p_normal)
{
  Vec4 normal, res;
  normal.Normalize(p_normal);
  res.w = normal.Dot(p_point1);
  res.x = normal.x;
  res.y = normal.y;
  res.z = normal.z;
  return res;
}
inline float
DistanceToPlane(const Vec4& point, const Vec4& plan)
{
  return plan.Dot3(point) + plan.w;
}

Vec4
PointOnSegment(const Vec4& point, const Vec4& vertPos1, const Vec4& vertPos2);

void
ComputeSnap(float* value, float snap);
void
ComputeSnap(Vec4& value, const float* snap);

void
ComputeFrustumPlanes(Vec4* frustum, const float* clip);

}
