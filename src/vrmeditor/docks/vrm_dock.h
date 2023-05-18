#pragma once
#include "gui.h"
#include <vrm/gltfroot.h>

class VrmDock
{
public:
  static void CreateVrm(const AddDockFunc& addDock,
                        std::string_view title,
                        const std::shared_ptr<libvrm::gltf::GltfRoot>& scene);
  // static void CreateExpression(const AddDockFunc& addDock,
  //                              std::string_view title,
  //                              const std::shared_ptr<libvrm::gltf::Scene>& scene);
};
