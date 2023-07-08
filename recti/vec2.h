#pragma once

struct ImVec2;
namespace recti {

struct Vec2
{
  float X = 0;
  float Y = 0;

  Vec2() {}
  Vec2(float x, float y)
    : X(x)
    , Y(y)
  {
  }

  //
  Vec2(const ImVec2& v);

  Vec2 operator-(const Vec2& rhs) const { return { X - rhs.X, Y - rhs.Y }; }
  Vec2 operator+(const Vec2& rhs) const { return { X + rhs.X, Y + rhs.Y }; }

  Vec2& operator/=(float rhs)
  {
    X /= rhs;
    Y /= rhs;
    return *this;
  }
  Vec2& operator*=(float rhs)
  {
    X *= rhs;
    Y *= rhs;
    return *this;
  }

  float Dot(const Vec2& rhs) const { return X * X + Y * Y; }

  float SqrLength() const { return Dot(*this); }
};

inline Vec2
Lerp(const Vec2& a, const Vec2& b, float t)
{
  return Vec2(a.X + (b.X - a.X) * t, a.Y + (b.Y - a.Y) * t);
}
inline Vec2
Lerp(const Vec2& a, const Vec2& b, const Vec2& t)
{
  return Vec2(a.X + (b.X - a.X) * t.X, a.Y + (b.Y - a.Y) * t.Y);
}

}
