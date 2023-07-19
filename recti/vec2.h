#pragma once

struct ImVec2;
namespace recti {

struct Vec2
{
  float x = 0;
  float y = 0;

  Vec2() {}
  Vec2(float _x, float _y)
    : x(_x)
    , y(_y)
  {
  }

  //
  Vec2(const ImVec2& v);

  Vec2 operator-(const Vec2& rhs) const { return { x - rhs.x, y - rhs.y }; }
  Vec2 operator+(const Vec2& rhs) const { return { x + rhs.x, y + rhs.y }; }

  Vec2& operator/=(float rhs)
  {
    x /= rhs;
    y /= rhs;
    return *this;
  }
  Vec2& operator*=(float rhs)
  {
    x *= rhs;
    y *= rhs;
    return *this;
  }

  float Dot(const Vec2& rhs) const { return x * x + y * y; }

  float SqrLength() const { return Dot(*this); }
};

inline Vec2
Lerp(const Vec2& a, const Vec2& b, float t)
{
  return Vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
}
inline Vec2
Lerp(const Vec2& a, const Vec2& b, const Vec2& t)
{
  return Vec2(a.x + (b.x - a.x) * t.x, a.y + (b.y - a.y) * t.y);
}

}
