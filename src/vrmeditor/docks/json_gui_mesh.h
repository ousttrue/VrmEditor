#pragma once
#include "json_gui.h"
#include "json_gui_table.h"
#include <imgui.h>

inline ShowGui
ShowSelected_meshes(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                    const libvrm::JsonPath& jsonpath)
{
  if (jsonpath.Size() == 1) {
    return [scene]() {
      auto meshes = scene->m_gltf.Json.at("meshes");
      std::array<const char*, 2> cols = {
        "index",
        "name",
      };
      if (JsonGuiTable("##meshes", cols)) {
        for (int i = 0; i < meshes.size(); ++i) {
          auto& mesh = meshes[i];
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("%d", i);
          ImGui::TableSetColumnIndex(1);
          ImGui::Text("%s", ((std::string)mesh["name"]).c_str());
        }
        ImGui::EndTable();
      }
    };
  } else if (jsonpath.Size() == 2) {
    if (auto _i = jsonpath.GetInt(1)) {
      auto i = *_i;
      return [scene,
              prims =
                scene->m_gltf.Json.at("meshes").at(i).at("primitives")]() {
        std::array<const char*, 4> cols = {
          "index",
          "vertices",
          "attrs",
          "indices",
        };
        if (JsonGuiTable("##prims", cols)) {
          for (int i = 0; i < prims.size(); ++i) {
            auto& prim = prims[i];
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", i);
            ImGui::TableSetColumnIndex(1);
            auto attributes = prim.at("attributes");
            int POSITION = attributes.at("POSITION");
            ImGui::Text(
              "%d",
              (int)scene->m_gltf.Json.at("accessors").at(POSITION).at("count"));
            ImGui::TableSetColumnIndex(2);
            std::stringstream ss;
            for (auto kv : attributes.items()) {
              ss << "," << kv.key();
            }
            ImGui::Text("%s", ss.str().c_str());
            ImGui::TableSetColumnIndex(3);
            int indices = prim.at("indices");
            ImGui::Text(
              "%d",
              (int)scene->m_gltf.Json.at("accessors").at(indices).at("count"));
          }
          ImGui::EndTable();
        }
      };
    }
  }
  return []() {};
}
