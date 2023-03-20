#pragma once
#include <functional>
#include <memory>
#include <vector>

struct Camera;
struct Mesh;
struct Node;
using RenderFunc = std::function<void(const Camera &, const Mesh &)>;

class Scene {
  std::vector<std::shared_ptr<Mesh>> m_meshes;
  std::vector<std::shared_ptr<Node>> m_nodes;
  std::vector<uint32_t> m_roots;

public:
  Scene() {}
  Scene(const Scene &) = delete;
  Scene &operator=(const Scene &) = delete;
  void load(const char *path);
  void render(const Camera &camera, const RenderFunc &render);
};
