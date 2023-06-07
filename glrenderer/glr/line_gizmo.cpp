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
                    const DirectX::XMFLOAT4& color)
{
  m_lines.push_back({
    .Position = { p0.x, p0.y, p0.z },
    .Color = color,
  });
  m_lines.push_back({
    .Position = { p1.x, p1.y, p1.z },
    .Color = color,
  });
}

void
LineGizmo::Arc(const DirectX::XMFLOAT4& color,
               const DirectX::XMFLOAT3& pos,
               const DirectX::XMFLOAT3& normal,
               const DirectX::XMFLOAT3& base,
               float delta,
               int count)
{
  auto Y = DirectX::XMLoadFloat3(&normal);
  auto Z = DirectX::XMLoadFloat3(&base);
  auto X = DirectX::XMVector3Cross(Y, Z);
  auto XX = DirectX::XMVector3Normalize(X);
  auto YY = DirectX::XMVector3Normalize(Y);
  auto ZZ = DirectX::XMVector3Normalize(Z);
  DirectX::XMFLOAT4 _{ 0, 0, 0, 1 };
  auto r = DirectX::XMMATRIX(XX, YY, ZZ, DirectX::XMLoadFloat4(&_));
  auto t = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
  auto m = r * t;

  auto begin = 0.0f;
  DirectX::XMFLOAT3 p0;
  DirectX::XMStoreFloat3(&p0, DirectX::XMVector3Transform(Z, m));
  for (int i = 0; i < count; ++i) {
    auto end = begin + delta;
    DirectX::XMFLOAT3 p1;
    DirectX::XMStoreFloat3(&p1,
                           DirectX::XMVector3Transform(
                             Z, DirectX::XMMatrixRotationAxis(YY, end) * m));

    DrawLine(p0, p1, color);

    // next
    p0 = p1;
    begin = end;
  }
}

void
LineGizmo::DrawSphere(const DirectX::XMFLOAT3& pos,
                      float r,
                      const DirectX::XMFLOAT4& color)
{
  CircleXY(pos, r, color);
  CircleYZ(pos, r, color);
  CircleZX(pos, r, color);
}

void
LineGizmo::DrawCapsule(const DirectX::XMFLOAT3& p0,
                       const DirectX::XMFLOAT3& p1,
                       float radius,
                       const DirectX::XMFLOAT4& color)
{
  DrawSphere(p0, radius, color);
  DrawSphere(p1, radius, color);
  DrawLine(p0, p1, color);
  // auto P0 = DirectX::XMLoadFloat3(&p0);
  // auto P1 = DirectX::XMLoadFloat3(&p1);
  // auto P01 = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(P1, P0));
  // DirectX::XMFLOAT3 z{ 0, 0, 1 };
  // auto Z = DirectX::XMLoadFloat3(&z);
  // auto dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(P01, Z));
  // if (fabs(fabs(dot) - 1) < 1e-4) {
  //
  // } else {
  //   DirectX::XMFLOAT3 x{ 1, 0, 0 };
  //   auto X = DirectX::XMLoadFloat3(&x);
  // }
  // Arc(color, p0, P01, )
}

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
//          *((DirectX::XMFLOAT4*)&color));
// DrawLine(dmath::add(pos, points[1]),
//          dmath::add(pos, points[2]),
//          *((DirectX::XMFLOAT4*)&color));
// DrawLine(dmath::add(pos, points[2]),
//          dmath::add(pos, points[3]),
//          *((DirectX::XMFLOAT4*)&color));
// DrawLine(dmath::add(pos, points[3]),
//          dmath::add(pos, points[4]),
//          *((DirectX::XMFLOAT4*)&color));
// DrawLine(dmath::add(pos, points[0]),
//          dmath::add(pos, points[4]),
//          *((DirectX::XMFLOAT4*)&color));
// DrawLine(dmath::add(pos, points[1]),
//          dmath::add(pos, points[4]),
//          *((DirectX::XMFLOAT4*)&color));
// DrawLine(dmath::add(pos, points[2]),
//          dmath::add(pos, points[4]),
//          *((DirectX::XMFLOAT4*)&color));
// DrawLine(dmath::add(pos, points[3]),
//          dmath::add(pos, points[4]),
//          *((DirectX::XMFLOAT4*)&color));
// DrawLine(dmath::add(pos, points[0]),
//          dmath::add(pos, points[5]),
//          *((DirectX::XMFLOAT4*)&color));
// DrawLine(dmath::add(pos, points[1]),
//          dmath::add(pos, points[5]),
//          *((DirectX::XMFLOAT4*)&color));
// DrawLine(dmath::add(pos, points[2]),
//          dmath::add(pos, points[5]),
//          *((DirectX::XMFLOAT4*)&color));
// DrawLine(dmath::add(pos, points[3]),
//          dmath::add(pos, points[5]),
//          *((DirectX::XMFLOAT4*)&color));

} // namespace gizmo
