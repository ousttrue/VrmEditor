#pragma once
#include "mat4.h"
#include "vec2.h"

namespace recti {

struct Camera
{
  Mat4 ViewMatrix;
  Mat4 ProjectionMatrix;
  Vec4 Viewport;
  Vec2 LeftTop() const { return { Viewport.x, Viewport.y }; }
  Vec2 Size() const { return { Viewport.z, Viewport.w }; }
  float Width() const { return Viewport.z; }
  float Height() const { return Viewport.w; }
  float Right() const { return Viewport.x + Viewport.z; }
  float Bottom() const { return Viewport.y + Viewport.w; }
  bool IsInContextRect(const recti::Vec2& p) const
  {
    return recti::IsWithin(p.X, Viewport.x, Right()) &&
           recti::IsWithin(p.Y, Viewport.y, Bottom());
  }
  float DisplayRatio() const { return Width() / Height(); }
};

struct Mouse
{
  Vec2 Position;
  bool LeftDown;
};

struct CameraMouse
{
  Camera Camera;
  Mouse Mouse;
private:
public:
  void Initialize(const struct Camera &camera, const struct Mouse &mouse)
  {
    Camera = camera;
    Mouse = mouse;
  }
};

} // namespace
