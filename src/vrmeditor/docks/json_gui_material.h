#pragma once
#include "json_gui.h"
#include <imgui.h>

inline ShowGui
JsonGuiMaterialList(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                       std::string_view jsonpath)
{
  return []() {
    //
    ImGui::TextUnformatted("materials");
  };
}
