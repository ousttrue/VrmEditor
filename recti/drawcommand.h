#pragma once
#include "style.h"
#include "vec2.h"
#include "vec4.h"
#include <array>
#include <optional>
#include <stdint.h>
#include <string>
#include <variant>
#include <vector>

namespace recti {

using VEC2 = Vec2;

struct DrawList
{
  struct Line
  {
    VEC2 p1;
    VEC2 p2;
  };
  struct Triangle
  {
    VEC2 p1;
    VEC2 p2;
    VEC2 p3;
  };
  struct Circle
  {
    VEC2 center;
    float radius;
    int num_segments;
  };
  struct Text
  {
    VEC2 pos;
    std::string text;
  };
  struct Polyline
  {
    std::vector<VEC2> points;
    int flags = 0;
  };

  struct DrawCommand
  {
    std::variant<Line, Triangle, Circle, Polyline, Text> m_shape;
    uint32_t m_col;
    std::optional<float> m_thickness;
  };

  std::vector<DrawCommand> m_commands;

  void AddLine(const VEC2& p1,
               const VEC2& p2,
               uint32_t col,
               float thickness = 1.0f)
  {
    m_commands.push_back({ Line{ p1, p2 }, col, thickness });
  }

  void AddTriangleFilled(const VEC2& p1,
                         const VEC2& p2,
                         const VEC2& p3,
                         uint32_t col)
  {
    m_commands.push_back({ Triangle{ p1, p2, p3 }, col });
  }

  void AddCircle(const VEC2& center,
                 float radius,
                 uint32_t col,
                 int num_segments = 0,
                 float thickness = 1.0f)
  {
    m_commands.push_back(
      { Circle{ center, radius, num_segments }, col, thickness });
  }

  void AddCircleFilled(const VEC2& center,
                       float radius,
                       uint32_t col,
                       int num_segments = 0)
  {
    m_commands.push_back({ Circle{ center, radius, num_segments }, col });
  }

  void AddText(const VEC2& pos,
               uint32_t col,
               const char* text_begin,
               const char* text_end =nullptr)
  {
    m_commands.push_back({ Text{ pos,
                                 text_end ? std::string{ text_begin, text_end }
                                          : std::string{ text_begin } },
                           col });
  }

  void AddPolyline(const VEC2* points,
                   int num_points,
                   uint32_t col,
                   int flags,
                   float thickness)
  {
    Polyline line;
    line.points.assign(points, points + num_points);
    line.flags = flags;
    m_commands.push_back({ line, col, thickness });
  }

  void AddConvexPolyFilled(const VEC2* points, int num_points, uint32_t col)
  {
    Polyline line;
    line.points.assign(points, points + num_points);
    m_commands.push_back({ line, col });
  }

  void DrawHatchedAxis(const struct ModelContext& mCurrent,
                       const Vec4& axis,
                       const Style& mStyle);

  void DrawCubes(const struct CameraMouse& cameraMouse,
                 const float* matrices,
                 int matrixCount,
                 const Style& mStyle);
};

} // namespace
