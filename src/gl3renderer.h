#pragma once

struct Camera;
struct Mesh;
class Gl3Renderer {

  class Gl3RendererImpl *m_impl;

public:
  Gl3Renderer();
  ~Gl3Renderer();
  void clear(const Camera &camera);
  void render(const Camera &camera, const Mesh &mesh, const float m[16]);
};
