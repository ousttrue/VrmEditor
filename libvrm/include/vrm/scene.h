#pragma once
#include "scenetypes.h"
#include <DirectXMath.h>
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <ostream>
#include <span>
#include <stdint.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct Camera;
struct Mesh;
struct Skin;
struct Node;
struct Animation;
class Image;
class Material;
using RenderFunc =
    std::function<void(const Camera &, const Mesh &, const float[16])>;

using EnterFunc = std::function<bool(Node &, const DirectX::XMFLOAT4X4 &)>;
using LeaveFunc = std::function<void(Node &)>;

struct Scene {
  std::vector<std::shared_ptr<Image>> m_images;
  std::vector<std::shared_ptr<Material>> m_materials;
  std::vector<std::shared_ptr<Mesh>> m_meshes;
  std::vector<std::shared_ptr<Node>> m_nodes;
  std::vector<std::shared_ptr<Node>> m_roots;
  std::vector<std::shared_ptr<Skin>> m_skins;
  std::vector<std::shared_ptr<Animation>> m_animations;

  Scene() {}
  Scene(const Scene &) = delete;
  Scene &operator=(const Scene &) = delete;
  void load(const char *path);
  void render(const Camera &camera, const RenderFunc &render,
              std::chrono::milliseconds time);
  void traverse(const EnterFunc &enter, const LeaveFunc &leave,
                Node *node = nullptr, const DirectX::XMFLOAT4X4 &parent = {});
};
