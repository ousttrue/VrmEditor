#pragma once
#include "line_gizmo.h"
#include <DirectXMath.h>
#include <cuber/gl3/GlCubeRenderer.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <memory>
#include <vector>
#include <vrm/node.h>

namespace glr {
class Cuber
{
  std::shared_ptr<cuber::gl3::GlCubeRenderer> m_cuber;
  std::shared_ptr<cuber::gl3::GlLineRenderer> m_liner;
  LineGizmo m_gizmo;

public:
  Cuber();
  std::vector<DirectX::XMFLOAT4X4> Instances;
  void Render(const struct RenderingEnv& camera);
};
}
