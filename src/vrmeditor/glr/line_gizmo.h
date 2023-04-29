#pragma once
#include <DirectXMath.h>
#include <cuber/mesh.h>
#include <vector>
#include <vrm/dmath.h>
#include <vrm/gizmo.h>

namespace glr {

struct LineGizmo : public libvrm::IGizmoDrawer
{
  std::vector<cuber::LineVertex> m_lines;
  size_t m_keep = 0;

  LineGizmo();
  void Fix() override { m_keep = m_lines.size(); }
  void Clear() override { m_lines.resize(m_keep); }

  void DrawLine(const DirectX::XMFLOAT3& p0,
                const DirectX::XMFLOAT3& p1,
                const DirectX::XMFLOAT4& color) override;
  void DrawSphere(const DirectX::XMFLOAT3& pos,
                  float radius,
                  const DirectX::XMFLOAT4& color) override;
  void DrawCapsule(const DirectX::XMFLOAT3& p0,
                   const DirectX::XMFLOAT3& p1,
                   float radius,
                   const DirectX::XMFLOAT4& color) override;

  void Arc(const DirectX::XMFLOAT4& color,
           const DirectX::XMFLOAT3& pos,
           const DirectX::XMFLOAT3& normal,
           const DirectX::XMFLOAT3& base,
           float delta,
           int count);

  void CircleXY(const DirectX::XMFLOAT3& pos,
                float r,
                const DirectX::XMFLOAT4& color,
                int division = 16)
  {
    Arc(color,
        pos,
        { 0, 0, 1 },
        { r, 0, 0 },
        DirectX::XM_2PI / division,
        division);
  }

  void CircleYZ(const DirectX::XMFLOAT3& pos,
                float r,
                const DirectX::XMFLOAT4& color,
                int division = 16)
  {
    Arc(color,
        pos,
        { 1, 0, 0 },
        { 0, r, 0 },
        DirectX::XM_2PI / division,
        division);
  }

  void CircleZX(const DirectX::XMFLOAT3& pos,
                float r,
                const DirectX::XMFLOAT4& color,
                int division = 16)
  {
    Arc(color,
        pos,
        { 0, 1, 0 },
        { 0, 0, r },
        DirectX::XM_2PI / division,
        division);
  }
};

}
