#pragma once
#include "json_gui.h"
#include <imgui.h>

inline ShowGui
ShowSelected_materials(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                       const libvrm::JsonPath& jsonpath)
{
  return []() {
    //
    ImGui::TextUnformatted("materials");
  };
}
