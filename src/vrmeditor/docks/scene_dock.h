#pragma once
#include "gui.h"
#include <memory>
#include <string_view>
#include <vrm/scene.h>

class SceneDock
{
public:
  static std::shared_ptr<libvrm::gltf::SceneContext> CreateTree(
    const AddDockFunc& addDock,
    std::string_view title,
    const std::shared_ptr<libvrm::gltf::Scene>& scene,
    float indent);
};
