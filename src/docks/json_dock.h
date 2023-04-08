#pragma once
#include "gui.h"
#include <string_view>
#include <vrm/scene.h>

class JsonDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     std::string_view title,
                     const std::shared_ptr<gltf::Scene>& scene,
                     float indent);
};
