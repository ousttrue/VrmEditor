#pragma once
#include <DirectXMath.h>
#include <cuber/gl3/GlCubeRenderer.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <grapho/camera/camera.h>
#include <memory>
#include <vector>
#include <vrm/gizmo.h>
#include <vrm/node.h>

namespace glr {

class Gizmo : public libvrm::IGizmoDrawer
{
  std::shared_ptr<cuber::gl3::GlCubeRenderer> m_cuber;
  std::shared_ptr<cuber::gl3::GlLineRenderer> m_liner;
  size_t m_keep = 0;

public:
  std::vector<cuber::LineVertex> Lines;
  Gizmo();
  std::vector<cuber::Instance> Instances;
  void Render(const grapho::camera::Camera& camera,
              bool ShowCube,
              bool ShowLine);

  void Fix() override { m_keep = Lines.size(); }
  void Clear() override { Lines.resize(m_keep); }

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
