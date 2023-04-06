#pragma once

struct ViewProjection;
namespace gltf {
struct Mesh;
struct MeshInstance;
} // namespace gltf
class Gl3Renderer
{

  class Gl3RendererImpl* m_impl;

public:
  Gl3Renderer();
  ~Gl3Renderer();
  void Release();
  void ClearRendertarget(const ViewProjection& camera);
  void Render(const ViewProjection& camera,
              const gltf::Mesh& mesh,
              const gltf::MeshInstance& instance,
              const float m[16]);
};
