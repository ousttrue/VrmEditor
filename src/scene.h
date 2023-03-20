#pragma once
#include <functional>

struct Camera;
struct Mesh;
using RenderFunc = std::function<void(const Camera &, const Mesh &)>;

class Scene {

  struct SceneImpl *m_impl;

public:
  Scene();
  ~Scene();
  void load(const char *path);
  void render(const Camera &camera, const RenderFunc &render);
};
