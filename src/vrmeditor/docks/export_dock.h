#pragma once
#include "gui.h"

namespace libvrm {
namespace gltf {
struct Scene;
}
struct Timeline;
}

class ExportDock
{
public:
  static void Create(const AddDockFunc& addDock,
                     std::string_view title,
                     const std::shared_ptr<libvrm::Timeline>& timeline,
                     const std::shared_ptr<libvrm::gltf::Scene>& scene,
                     float indent);
};
