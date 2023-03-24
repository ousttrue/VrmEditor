#pragma once
#include "scenetypes.h"
#include <DirectXMath.h>
#include <chrono>
#include <filesystem>
#include <functional>
#include <list>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <ostream>
#include <span>
#include <stdint.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;

struct Camera;
struct Mesh;
struct Skin;
struct Node;
struct Animation;
class Image;
class Material;
struct Vrm0;
using RenderFunc =
    std::function<void(const Camera &, const Mesh &, const float[16])>;

using EnterFunc = std::function<bool(Node &, const DirectX::XMFLOAT4X4 &)>;
using LeaveFunc = std::function<void()>;
using EnterJson = std::function<bool(json &, const std::string &key)>;
using LeaveJson = std::function<void()>;

struct Scene {
  std::vector<std::shared_ptr<Image>> m_images;
  std::vector<std::shared_ptr<Material>> m_materials;
  std::vector<std::shared_ptr<Mesh>> m_meshes;
  std::vector<std::shared_ptr<Node>> m_nodes;
  std::vector<std::shared_ptr<Node>> m_roots;
  std::vector<std::shared_ptr<Skin>> m_skins;
  std::vector<std::shared_ptr<Animation>> m_animations;
  json m_gltf;
  std::shared_ptr<Vrm0> m_vrm0;

  void clear() {
    m_images.clear();
    m_materials.clear();
    m_meshes.clear();
    m_nodes.clear();
    m_roots.clear();
    m_skins.clear();
    m_animations.clear();
    m_gltf = {};
  }

  Scene() {}
  Scene(const Scene &) = delete;
  Scene &operator=(const Scene &) = delete;
  bool load(const std::filesystem::path &path);

private:
  void addIndices(int vertex_offset, Mesh *mesh, struct Glb *glb,
                  int accessor_index, int material_index);

public:
  void render(const Camera &camera, const RenderFunc &render,
              std::chrono::milliseconds time);
  void traverse(const EnterFunc &enter, const LeaveFunc &leave,
                Node *node = nullptr, const DirectX::XMFLOAT4X4 &parent = {});
  void traverse_json(const EnterJson &enter, const LeaveJson &leave,
                     json *j = nullptr, std::string_view key = {});
};
