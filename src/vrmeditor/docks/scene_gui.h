#pragma once
#include <map>
#include <memory>
#include <string>

namespace libvrm {
namespace gltf {
struct Material;
struct Node;
}
}

namespace runtimescene {
struct RuntimeScene;
}

using NodeWeakPtr = std::weak_ptr<libvrm::gltf::Node>;

struct SceneNodeSelection;

class SceneGui
{
  std::map<NodeWeakPtr, std::string, std::owner_less<NodeWeakPtr>> m_map;
  std::shared_ptr<runtimescene::RuntimeScene> m_scene;
  std::shared_ptr<SceneNodeSelection> m_selection;
  float m_indent;

public:
  SceneGui(const std::shared_ptr<runtimescene::RuntimeScene>& scene,
           const std::shared_ptr<SceneNodeSelection>& selection,
           float indent);
  void Show(const char* title, bool* p_open);
  bool Enter(const std::shared_ptr<libvrm::gltf::Node>& node);
  void Leave();
  const std::string& Label(const std::shared_ptr<libvrm::gltf::Node>& node);

private:
  void ShowMaterial(int i,
                    const std::shared_ptr<libvrm::gltf::Material>& material);
  void ShowNodes();
};
