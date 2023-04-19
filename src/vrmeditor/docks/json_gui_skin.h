#pragma once
#include "json_gui.h"
#include "json_gui_table.h"
#include <imgui.h>

inline ShowGui
JsonGuiSkinList(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                std::string_view jsonpath)
{
  return [scene]() {
    auto skins = scene->m_gltf.Json.at("skins");
    std::array<const char*, 2> cols = {
      "index",
      "joints",
    };
    if (JsonGuiTable("##skins", cols)) {
      for (int i = 0; i < skins.size(); ++i) {
        auto& skin = skins[i];
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", i);

        ImGui::TableSetColumnIndex(1);
        int j = 0;
        std::stringstream ss;
        for (auto joint : skin.at("joints")) {
          if (j++) {
            ss << ',';
          }
          ss << joint;
        }
        ImGui::Text("%s", ss.str().c_str());
      }
      ImGui::EndTable();
    }
  };
}
