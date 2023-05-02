#pragma once
#include "gui.h"
#include <memory>
#include <runtimescene/scene.h>
#include <string_view>

class SceneDock
{
public:
  static void CreateTree(
    const AddDockFunc& addDock,
    std::string_view title,
    const std::shared_ptr<runtimescene::RuntimeScene>& scene,
    float indent);
};
