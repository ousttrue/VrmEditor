#pragma once
#include "json_gui.h"
#include "json_gui_table.h"
#include <imgui.h>

inline ShowGui
ShowSelected_accessors(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                       std::string_view jsonpath)
{
  if (auto accessor_index = libvrm::JsonPath(jsonpath).GetInt(1)) {
    auto accessor = scene->m_gltf.Json.at("accessors").at(*accessor_index);
    if (accessor.at("type") == "VEC3" && accessor.at("componentType") == 5126) {
      // float3 table
      if (auto values =
            scene->m_gltf.accessor<DirectX::XMFLOAT3>(*accessor_index)) {

        auto items = *values;
        return [items]() {
          ImGui::Text("float3[%zu]", items.size());
          std::array<const char*, 4> cols = {
            "index",
            "x",
            "y",
            "z",
          };
          if (JsonGuiTable("##accessor_values", cols)) {
            ImGuiListClipper clipper;
            clipper.Begin(items.size());
            while (clipper.Step()) {
              for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                auto& value = items[i];
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", i);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%f", value.x);
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%f", value.y);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%f", value.z);
              }
            }
            ImGui::EndTable();
          }
        };
      }
    }
    int count = accessor.at("count");
    return [count]() { ImGui::Text("%d", count); };
  }

  return []() {};
}
