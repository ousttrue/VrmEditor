#pragma once

struct matrix_t;
struct ImVec2;
struct vec_t
{
  float x = 0;
  float y = 0;
  float z = 0;
  float w = 0;

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
  float Length() const;
  float LengthSq() const { return (x * x + y * y + z * z); };
  vec_t Normalize();
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
  bool operator!=(const vec_t& other) const;
};

vec_t
makeVect(const ImVec2& v);

inline vec_t
vec_t::operator*(float f) const
{
  return { x * f, y * f, z * f, w * f };
}

inline vec_t
vec_t::operator-() const
{
  return { -x, -y, -z, -w };
}

inline vec_t
vec_t::operator-(const vec_t& v) const
{
  return { x - v.x, y - v.y, z - v.z, w - v.w };
}

inline vec_t
vec_t::operator+(const vec_t& v) const
{
  return { x + v.x, y + v.y, z + v.z, w + v.w };
}

inline vec_t
vec_t::operator*(const vec_t& v) const
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

inline vec_t
Normalized(const vec_t& v)
{
  vec_t res;
  res = v;
  res.Normalize();
  return res;
}

inline vec_t
Cross(const vec_t& v1, const vec_t& v2)
{
  vec_t res;
  res.x = v1.y * v2.z - v1.z * v2.y;
  res.y = v1.z * v2.x - v1.x * v2.z;
  res.z = v1.x * v2.y - v1.y * v2.x;
  res.w = 0.f;
  return res;
}

inline float
Dot(const vec_t& v1, const vec_t& v2)
{
  return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

inline vec_t
BuildPlan(const vec_t& p_point1, const vec_t& p_normal)
{
  vec_t normal, res;
  normal.Normalize(p_normal);
  res.w = normal.Dot(p_point1);
  res.x = normal.x;
  res.y = normal.y;
  res.z = normal.z;
  return res;
}
inline float
DistanceToPlane(const vec_t& point, const vec_t& plan)
{
  return plan.Dot3(point) + plan.w;
}

float
IntersectRayPlane(const vec_t& rOrigin,
                  const vec_t& rVector,
                  const vec_t& plan);

vec_t
PointOnSegment(const vec_t& point,
               const vec_t& vertPos1,
               const vec_t& vertPos2);

void
ComputeSnap(float* value, float snap);
void
ComputeSnap(vec_t& value, const float* snap);

void
ComputeFrustumPlanes(vec_t* frustum, const float* clip);
