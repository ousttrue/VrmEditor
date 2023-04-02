#pragma once

struct Camera;
namespace gltf {
struct Mesh;
struct MeshInstance;
} // namespace gltf
class Gl3Renderer {

  class Gl3RendererImpl *m_impl;

public:
  Gl3Renderer();
  ~Gl3Renderer();
  void clear(const Camera &camera);
  void render(const Camera &camera, const gltf::Mesh &mesh,
              const gltf::MeshInstance &instance, const float m[16]);
};
