#pragma once
#include "json_gui.h"
#include "json_gui_table.h"
#include <imgui.h>

inline ShowGui
ShowSelected_accessors(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                       const libvrm::JsonPath& jsonpath)
{
  if (jsonpath.Size() == 2) {
    if (auto _i = jsonpath.GetInt(1)) {
      auto i = *_i;
      auto accessor = scene->m_gltf.Json.at("accessors").at(i);
      if (accessor.at("type") == "VEC3" &&
          accessor.at("componentType") == 5126) {
        // float3 table
        if (auto values = scene->m_gltf.accessor<DirectX::XMFLOAT3>(i)) {

          return [items = *values]() {
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
                for (int row_n = clipper.DisplayStart;
                     row_n < clipper.DisplayEnd;
                     row_n++) {
                  auto& value = items[row_n];
                  ImGui::TableNextRow();
                  ImGui::TableSetColumnIndex(0);
                  ImGui::Text("%d", row_n);
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
  }
  return []() { ImGui::TextUnformatted("accessors[x] fail"); };
}
