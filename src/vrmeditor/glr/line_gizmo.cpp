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
LineGizmo::CircleXY(const DirectX::XMMATRIX& m,
                    float r,
                    const grapho::RGBA& color,
                    int division)
{
  auto delta = DirectX::XM_2PI / division;
  auto begin = 0.0f;
  auto _p = DirectX::XMFLOAT3{ r, 0, 0 };
  auto p = DirectX::XMLoadFloat3(&_p);
  DirectX::XMFLOAT3 p0;
  DirectX::XMStoreFloat3(&p0, DirectX::XMVector3Transform(p, m));
  for (int i = 0; i < division; ++i) {
    auto end = begin + delta;
    DirectX::XMFLOAT3 p1;
    DirectX::XMStoreFloat3(
      &p1, DirectX::XMVector3Transform(p, DirectX::XMMatrixRotationZ(end) * m));

    DrawLine(p0, p1, color);

    // next
    p0 = p1;
    begin = end;
  }
}
void
LineGizmo::CircleYZ(const DirectX::XMMATRIX& m,
                    float r,
                    const grapho::RGBA& color,
                    int division)
{
  auto delta = DirectX::XM_2PI / division;
  auto begin = 0.0f;
  auto _p = DirectX::XMFLOAT3{ 0, r, 0 };
  auto p = DirectX::XMLoadFloat3(&_p);
  DirectX::XMFLOAT3 p0;
  DirectX::XMStoreFloat3(&p0, DirectX::XMVector3Transform(p, m));
  for (int i = 0; i < division; ++i) {
    auto end = begin + delta;
    DirectX::XMFLOAT3 p1;
    DirectX::XMStoreFloat3(
      &p1, DirectX::XMVector3Transform(p, DirectX::XMMatrixRotationX(end) * m));

    DrawLine(p0, p1, color);

    // next
    p0 = p1;
    begin = end;
  }
}
void
LineGizmo::CircleZX(const DirectX::XMMATRIX& m,
                    float r,
                    const grapho::RGBA& color,
                    int division)
{
  auto delta = DirectX::XM_2PI / division;
  auto begin = 0.0f;
  auto _p = DirectX::XMFLOAT3{ 0, 0, r };
  auto p = DirectX::XMLoadFloat3(&_p);
  DirectX::XMFLOAT3 p0;
  DirectX::XMStoreFloat3(&p0, DirectX::XMVector3Transform(p, m));
  for (int i = 0; i < division; ++i) {
    auto end = begin + delta;
    DirectX::XMFLOAT3 p1;
    DirectX::XMStoreFloat3(
      &p1, DirectX::XMVector3Transform(p, DirectX::XMMatrixRotationY(end) * m));

    DrawLine(p0, p1, color);

    // next
    p0 = p1;
    begin = end;
  }
}

void
LineGizmo::DrawSphere(const DirectX::XMFLOAT3& pos,
                      float r,
                      const grapho::RGBA& color)
{
  CircleXY(DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z), r, color);
  CircleYZ(DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z), r, color);
  CircleZX(DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z), r, color);
  // DirectX::XMFLOAT3 points[]{
  //   { r, 0, 0 }, { 0, 0, -r }, { -r, 0, 0 },
  //   { 0, 0, r }, { 0, r, 0 },  { 0, -r, 0 },
  // };
  // //  /\
  // // /  \
  // //+----+
  // // \  /
  // //  \/
  // DrawLine(dmath::add(pos, points[0]),
  //          dmath::add(pos, points[1]),
  //          *((grapho::RGBA*)&color));
  // DrawLine(dmath::add(pos, points[1]),
  //          dmath::add(pos, points[2]),
  //          *((grapho::RGBA*)&color));
  // DrawLine(dmath::add(pos, points[2]),
  //          dmath::add(pos, points[3]),
  //          *((grapho::RGBA*)&color));
  // DrawLine(dmath::add(pos, points[3]),
  //          dmath::add(pos, points[4]),
  //          *((grapho::RGBA*)&color));
  // DrawLine(dmath::add(pos, points[0]),
  //          dmath::add(pos, points[4]),
  //          *((grapho::RGBA*)&color));
  // DrawLine(dmath::add(pos, points[1]),
  //          dmath::add(pos, points[4]),
  //          *((grapho::RGBA*)&color));
  // DrawLine(dmath::add(pos, points[2]),
  //          dmath::add(pos, points[4]),
  //          *((grapho::RGBA*)&color));
  // DrawLine(dmath::add(pos, points[3]),
  //          dmath::add(pos, points[4]),
  //          *((grapho::RGBA*)&color));
  // DrawLine(dmath::add(pos, points[0]),
  //          dmath::add(pos, points[5]),
  //          *((grapho::RGBA*)&color));
  // DrawLine(dmath::add(pos, points[1]),
  //          dmath::add(pos, points[5]),
  //          *((grapho::RGBA*)&color));
  // DrawLine(dmath::add(pos, points[2]),
  //          dmath::add(pos, points[5]),
  //          *((grapho::RGBA*)&color));
  // DrawLine(dmath::add(pos, points[3]),
  //          dmath::add(pos, points[5]),
  //          *((grapho::RGBA*)&color));
}

} // namespace gizmo
