#pragma once

namespace gltf {
struct ViewProjection;
struct Mesh;
struct MeshInstance;
} // namespace gltf
class Gl3Renderer
{

  class Gl3RendererImpl* m_impl;

public:
  Gl3Renderer();
  ~Gl3Renderer();
  void clear(const gltf::ViewProjection& camera);
  void render(const gltf::ViewProjection& camera,
              const gltf::Mesh& mesh,
              const gltf::MeshInstance& instance,
              const float m[16]);
};
