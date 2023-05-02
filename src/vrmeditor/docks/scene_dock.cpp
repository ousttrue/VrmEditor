#include "scene_dock.h"
#include "scene_gui.h"
#include <imgui.h>
#include <memory>
#include <vrm/mesh.h>
#include <vrm/node.h>

void
SceneDock::CreateTree(const AddDockFunc& addDock,
                      std::string_view title,
                      const std::shared_ptr<runtimescene::RuntimeScene>& scene,
                      const std::shared_ptr<SceneNodeSelection>& selection,
                      float indent)
{
  auto gui = std::make_shared<SceneGui>(scene, selection, indent);

  addDock(Dock(title, [gui](const char* title, bool* p_open) {
    gui->Show(title, p_open);
  }));
}
