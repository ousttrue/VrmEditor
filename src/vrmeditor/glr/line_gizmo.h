#pragma once
#include <DirectXMath.h>
#include <grapho/mesh.h>
#include <vector>
#include <vrm/dmath.h>
#include <vrm/gizmo.h>

namespace glr {

struct LineGizmo : public libvrm::IGizmoDrawer
{
  std::vector<grapho::LineVertex> m_lines;
  size_t m_keep = 0;

  LineGizmo();
  void Fix() override { m_keep = m_lines.size(); }
  void Clear() override { m_lines.resize(m_keep); }

  void DrawLine(const DirectX::XMFLOAT3& p0,
                const DirectX::XMFLOAT3& p1,
                const grapho::RGBA& color) override;
  void DrawSphere(const DirectX::XMFLOAT3& pos,
                  const grapho::RGBA& color) override;
};

}
