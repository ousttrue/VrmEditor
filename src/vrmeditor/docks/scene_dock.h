#pragma once
#include "gui.h"
#include <memory>
#include <string_view>
#include <vrm/runtimescene/scene.h>

struct SceneNodeSelection;
class SceneDock
{
public:
  static void CreateTree(
    const AddDockFunc& addDock,
    std::string_view title,
    const std::shared_ptr<runtimescene::RuntimeScene>& scene,
    const std::shared_ptr<SceneNodeSelection>& selection,
    float indent);
};
