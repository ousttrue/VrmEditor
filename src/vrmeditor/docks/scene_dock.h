#pragma once
#include "gui.h"
#include "treecontext.h"
#include <vrm/scene.h>
#include <memory>
#include <string_view>

class SceneDock
{
public:
  static std::shared_ptr<TreeContext> CreateTree(
    const AddDockFunc& addDock,
    std::string_view title,
    const std::shared_ptr<gltf::Scene>& scene,
    float indent);
};
