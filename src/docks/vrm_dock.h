#pragma once
#include "gui.h"
#include <vrm/scene.h>

class VrmDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     std::string_view title,
                     const std::shared_ptr<gltf::Scene>& scene);
};
