#pragma once
#include "scenetypes.h"
#include <DirectXMath.h>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <ostream>
#include <stdint.h>
#include <string>
#include <string_view>
#include <vector>

struct Camera;
struct Mesh;

struct Node : public std::enable_shared_from_this<Node> {
  uint32_t index;
  std::string name;
  float3 translation = {};
  quaternion rotation = {};
  float3 scale = {};
  std::optional<uint32_t> mesh;

  std::list<std::shared_ptr<Node>> children;
  std::weak_ptr<Node> parent;

  Node(uint32_t i, std::string_view name);
  Node(const Node &) = delete;
  Node &operator=(const Node &) = delete;
  void addChild(const std::shared_ptr<Node> &child);
  DirectX::XMFLOAT4X4 world(const DirectX::XMFLOAT4X4 &parent) const;
  void setWorldMatrix(const DirectX::XMFLOAT4X4 &world,
                      const DirectX::XMFLOAT4X4 &parent);
  void print(int level = 0);
};
inline std::ostream &operator<<(std::ostream &os, const Node &node) {
  os << "Node[" << node.index << "]" << node.name << ": " << node.translation
     << node.rotation << node.scale;
  if (node.mesh) {
    os << ", mesh: " << *node.mesh;
  }
  return os;
}

class Image;
class Material;
using RenderFunc =
    std::function<void(const Camera &, const Mesh &, const float[16])>;

using EnterFunc = std::function<bool(Node &, const DirectX::XMFLOAT4X4 &)>;
using LeaveFunc = std::function<void(Node &)>;

class Scene {
  std::vector<std::shared_ptr<Image>> m_images;
  std::vector<std::shared_ptr<Material>> m_materials;
  std::vector<std::shared_ptr<Mesh>> m_meshes;
  std::vector<std::shared_ptr<Node>> m_nodes;
  std::vector<std::shared_ptr<Node>> m_roots;

public:
  Scene() {}
  Scene(const Scene &) = delete;
  Scene &operator=(const Scene &) = delete;
  void load(const char *path);
  void render(const Camera &camera, const RenderFunc &render);
  void traverse(const EnterFunc &enter, const LeaveFunc &leave,
                Node *node = nullptr, const DirectX::XMFLOAT4X4 &parent = {});

private:
  void traverse(const Camera &camera, const RenderFunc &render,
                const std::shared_ptr<Node> &node,
                const DirectX::XMFLOAT4X4 &parent);
};
