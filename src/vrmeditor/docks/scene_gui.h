#pragma once
#include <map>
#include <memory>
#include <string>

namespace libvrm {
namespace gltf {
struct Scene;
struct SceneContext;
struct Node;
}
}
using NodeWeakPtr = std::weak_ptr<libvrm::gltf::Node>;

class SceneGui
{
  std::map<NodeWeakPtr, std::string, std::owner_less<NodeWeakPtr>> m_map;
  bool m_enableSpring = true;
  std::shared_ptr<libvrm::gltf::Scene> m_scene;
  float m_indent;

public:
  std::shared_ptr<libvrm::gltf::SceneContext> Context;
  SceneGui(const std::shared_ptr<libvrm::gltf::Scene>& scene, float indent);
  void Show(const char* title, bool* p_open);
  bool Enter(const std::shared_ptr<libvrm::gltf::Node>& node);
  void Leave();
  const std::string& Label(const libvrm::gltf::Scene& scene,
                           const std::shared_ptr<libvrm::gltf::Node>& node);
};
