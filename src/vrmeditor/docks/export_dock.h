#pragma once
#include "gui.h"

namespace runtimescene {
struct RuntimeScene;
}

class ExportDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     std::string_view title,
                     const std::shared_ptr<runtimescene::RuntimeScene>& scene,
                     float indent);
};
