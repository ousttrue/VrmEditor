#pragma once
#include "dockspace.h"
#include <vrm/runtime_scene.h>

class VrmDock
{
public:
  static void CreateVrm(const AddDockFunc& addDock,
                        std::string_view title,
                        const std::shared_ptr<libvrm::RuntimeScene>& scene);
  // static void CreateExpression(const AddDockFunc& addDock,
  //                              std::string_view title,
  //                              const std::shared_ptr<libvrm::gltf::Scene>&
  //                              scene);
};
