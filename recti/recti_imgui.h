#pragma once
#include "drawcommand.h"
#include "vec2.h"
#include <imgui.h>

struct ImDrawList;

namespace recti {

Vec2::Vec2(const ImVec2& v)
  : x(v.x)
  , y(v.y)
{
}

inline void
Render(const DrawList& list, ImDrawList* drawlist)
{
  using LINE = DrawList::Line;
  using TRIANGLE = DrawList::Triangle;
  using CIRCLE = DrawList::Circle;
  using POLYLINE = DrawList::Polyline;
  using TEXT = DrawList::Text;

  static_assert(sizeof(VEC2) == sizeof(ImVec2), "VEC2 size");

  struct Visitor
  {
    ImDrawList* mDrawList;
    uint32_t mColor;
    std::optional<float> mThickness;

    static inline const ImVec2& IM(const VEC2& p)
    {
      return *((const ImVec2*)&p);
    };

    void operator()(const LINE& shape)
    {
      if (mThickness) {
        mDrawList->AddLine(IM(shape.p1), IM(shape.p2), mColor, *mThickness);
      } else {
      }
    }
    void operator()(const TRIANGLE& shape)
    {
      if (mThickness) {
      } else {
        mDrawList->AddTriangleFilled(
          IM(shape.p1), IM(shape.p2), IM(shape.p2), mColor);
      }
    }
    void operator()(const CIRCLE& shape)
    {
      if (mThickness) {
        mDrawList->AddCircle(IM(shape.center),
                             shape.radius,
                             mColor,
                             shape.num_segments,
                             *mThickness);
      } else {
        mDrawList->AddCircleFilled(
          IM(shape.center), shape.radius, mColor, shape.num_segments);
      }
    }
    void operator()(const POLYLINE& shape)
    {
      if (mThickness) {
        mDrawList->AddPolyline((const ImVec2*)shape.points.data(),
                               shape.points.size(),
                               mColor,
                               0,
                               *mThickness);
      } else {
        mDrawList->AddConvexPolyFilled(
          (const ImVec2*)shape.points.data(), shape.points.size(), mColor);
      }
    }
    void operator()(const TEXT& shape)
    {
      mDrawList->AddText(IM(shape.pos),
                         mColor,
                         shape.text.data(),
                         shape.text.data() + shape.text.size());
    }
  };
  for (auto& c : list.m_commands) {
    std::visit(Visitor{ drawlist, c.m_col, c.m_thickness }, c.m_shape);
  }
  // list->m_commands.clear();
}
} // recti
