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
  float m_scaling = 1.0f;

public:
  Cuber();
  void CalcShape(const std::shared_ptr<gltf::Node>& node, float scaling);
  std::vector<DirectX::XMFLOAT4X4> Instances;
  void Update(const gltf::Scene& scene);
  void Render(const struct ViewProjection& camera);

private:
  void UpdateRecursive(const std::shared_ptr<gltf::Node>& node,
                       DirectX::XMMATRIX parent);
};
