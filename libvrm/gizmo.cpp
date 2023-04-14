#include "vrm/gizmo.h"
#include "vrm/dmath.h"

std::vector<grapho::LineVertex> g_lines;
size_t g_keep = 0;

namespace libvrm::gizmo {

std::vector<grapho::LineVertex> &lines() { return g_lines; }
void fix() { g_keep = g_lines.size(); }
void clear() { g_lines.resize(g_keep); }

void drawLine(const DirectX::XMFLOAT3 &p0, const DirectX::XMFLOAT3 &p1,
              const grapho::RGBA &color) {

  g_lines.push_back({
      .position = {p0.x, p0.y, p0.z},
      .color = color,
  });
  g_lines.push_back({
      .position = {p1.x, p1.y, p1.z},
      .color = color,
  });
}

void drawSphere(const DirectX::XMFLOAT3 &pos, const DirectX::XMFLOAT4 &color) {

  float r = 0.01f;

  static DirectX::XMFLOAT3 points[]{
      {r, 0, 0}, {0, 0, -r}, {-r, 0, 0}, {0, 0, r}, {0, r, 0}, {0, -r, 0},
  };
  //  /\
  // /  \
  //+----+
  // \  /
  //  \/
  drawLine(dmath::add(pos, points[0]), dmath::add(pos, points[1]),
           *((grapho::RGBA *)&color));
  drawLine(dmath::add(pos, points[1]), dmath::add(pos, points[2]),
           *((grapho::RGBA *)&color));
  drawLine(dmath::add(pos, points[2]), dmath::add(pos, points[3]),
           *((grapho::RGBA *)&color));
  drawLine(dmath::add(pos, points[3]), dmath::add(pos, points[4]),
           *((grapho::RGBA *)&color));
  drawLine(dmath::add(pos, points[0]), dmath::add(pos, points[4]),
           *((grapho::RGBA *)&color));
  drawLine(dmath::add(pos, points[1]), dmath::add(pos, points[4]),
           *((grapho::RGBA *)&color));
  drawLine(dmath::add(pos, points[2]), dmath::add(pos, points[4]),
           *((grapho::RGBA *)&color));
  drawLine(dmath::add(pos, points[3]), dmath::add(pos, points[4]),
           *((grapho::RGBA *)&color));
  drawLine(dmath::add(pos, points[0]), dmath::add(pos, points[5]),
           *((grapho::RGBA *)&color));
  drawLine(dmath::add(pos, points[1]), dmath::add(pos, points[5]),
           *((grapho::RGBA *)&color));
  drawLine(dmath::add(pos, points[2]), dmath::add(pos, points[5]),
           *((grapho::RGBA *)&color));
  drawLine(dmath::add(pos, points[3]), dmath::add(pos, points[5]),
           *((grapho::RGBA *)&color));
}

} // namespace gizmo
