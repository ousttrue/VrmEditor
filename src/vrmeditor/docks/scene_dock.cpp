#include "scene_dock.h"
#include "scene_node_label.h"
#include <imgui.h>
#include <memory>
#include <vrm/mesh.h>
#include <vrm/node.h>

std::shared_ptr<libvrm::gltf::SceneContext>
SceneDock::CreateTree(const AddDockFunc& addDock,
                      std::string_view title,
                      const std::shared_ptr<libvrm::gltf::Scene>& scene,
                      float indent)
{
  auto gui = std::make_shared<SceneGui>(scene, indent);

  addDock(Dock(title, [gui](const char* title, bool* p_open) {
    gui->Show(title, p_open);
  }));

  return gui->Context;
}
