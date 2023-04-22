#include "line_gizmo.h"
#include <cuber/mesh.h>

namespace glr {

LineGizmo::LineGizmo()
{
  cuber::PushGrid(m_lines);
  Fix();
}

void
LineGizmo::DrawLine(const DirectX::XMFLOAT3& p0,
                    const DirectX::XMFLOAT3& p1,
                    const grapho::RGBA& color)
{
  m_lines.push_back({
    .position = { p0.x, p0.y, p0.z },
    .color = color,
  });
  m_lines.push_back({
    .position = { p1.x, p1.y, p1.z },
    .color = color,
  });
}

void
LineGizmo::DrawSphere(const DirectX::XMFLOAT3& pos, const grapho::RGBA& color)
{

  float r = 0.01f;

  static DirectX::XMFLOAT3 points[]{
    { r, 0, 0 }, { 0, 0, -r }, { -r, 0, 0 },
    { 0, 0, r }, { 0, r, 0 },  { 0, -r, 0 },
  };
  //  /\
  // /  \
  //+----+
  // \  /
  //  \/
  DrawLine(dmath::add(pos, points[0]),
           dmath::add(pos, points[1]),
           *((grapho::RGBA*)&color));
  DrawLine(dmath::add(pos, points[1]),
           dmath::add(pos, points[2]),
           *((grapho::RGBA*)&color));
  DrawLine(dmath::add(pos, points[2]),
           dmath::add(pos, points[3]),
           *((grapho::RGBA*)&color));
  DrawLine(dmath::add(pos, points[3]),
           dmath::add(pos, points[4]),
           *((grapho::RGBA*)&color));
  DrawLine(dmath::add(pos, points[0]),
           dmath::add(pos, points[4]),
           *((grapho::RGBA*)&color));
  DrawLine(dmath::add(pos, points[1]),
           dmath::add(pos, points[4]),
           *((grapho::RGBA*)&color));
  DrawLine(dmath::add(pos, points[2]),
           dmath::add(pos, points[4]),
           *((grapho::RGBA*)&color));
  DrawLine(dmath::add(pos, points[3]),
           dmath::add(pos, points[4]),
           *((grapho::RGBA*)&color));
  DrawLine(dmath::add(pos, points[0]),
           dmath::add(pos, points[5]),
           *((grapho::RGBA*)&color));
  DrawLine(dmath::add(pos, points[1]),
           dmath::add(pos, points[5]),
           *((grapho::RGBA*)&color));
  DrawLine(dmath::add(pos, points[2]),
           dmath::add(pos, points[5]),
           *((grapho::RGBA*)&color));
  DrawLine(dmath::add(pos, points[3]),
           dmath::add(pos, points[5]),
           *((grapho::RGBA*)&color));
}

} // namespace gizmo
