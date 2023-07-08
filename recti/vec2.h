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
};

}
