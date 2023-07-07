#pragma once

struct matrix_t;
struct ImVec2;
struct vec_t
{
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

vec_t
makeVect(const ImVec2& v);

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
