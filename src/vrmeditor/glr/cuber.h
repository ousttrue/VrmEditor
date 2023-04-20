#pragma once
#include <DirectXMath.h>
#include <cuber/gl3/GlCubeRenderer.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <memory>
#include <vector>
#include <vrm/node.h>

class Cuber
{
  std::shared_ptr<cuber::gl3::GlCubeRenderer> m_cuber;
  std::shared_ptr<cuber::gl3::GlLineRenderer> m_liner;
  std::vector<grapho::LineVertex> m_lines;

public:
  Cuber();
  std::vector<DirectX::XMFLOAT4X4> Instances;
  void Render(const struct RenderingEnv& camera);
};
