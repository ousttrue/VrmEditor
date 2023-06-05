#pragma once
#include "gui.h"

namespace libvrm {
struct RuntimeScene;
}

class ExportDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     std::string_view title,
                     const std::shared_ptr<libvrm::RuntimeScene>& scene,
                     float indent);
};
