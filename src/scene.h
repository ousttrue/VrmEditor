#pragma once
#include <DirectXMath.h>
#include <functional>
#include <memory>
#include <vector>

struct Camera;
struct Mesh;
struct Node;
using RenderFunc =
    std::function<void(const Camera &, const Mesh &, const float[16])>;

class Scene {
  std::vector<std::shared_ptr<Mesh>> m_meshes;
  std::vector<std::shared_ptr<Node>> m_nodes;
  std::vector<std::shared_ptr<Node>> m_roots;

public:
  Scene() {}
  Scene(const Scene &) = delete;
  Scene &operator=(const Scene &) = delete;
  void load(const char *path);
  void render(const Camera &camera, const RenderFunc &render);

private:
  void traverse(const Camera &camera, const RenderFunc &render,
                const std::shared_ptr<Node> &node,
                const DirectX::XMFLOAT4X4 &parent);
};
