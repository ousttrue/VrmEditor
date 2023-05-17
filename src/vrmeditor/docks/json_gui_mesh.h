#pragma once
#include "json_gui.h"
#include "json_gui_table.h"
#include <imgui.h>

inline ShowGui
JsonGuiMeshList(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                std::string_view jsonpath)
{
  return [scene]() {
    std::array<const char*, 2> cols = {
      "index",
      "name",
    };
    auto& meshes = scene->m_gltf.Meshes;
    if (JsonGuiTable("##meshes", cols)) {
      for (int i = 0; i < meshes.Size(); ++i) {
        auto& mesh = meshes[i];
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", i);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", mesh.Name.c_str());
      }
      ImGui::EndTable();
    }
  };
}

inline ShowGui
JsonGuiMesh(const std::shared_ptr<libvrm::gltf::Scene>& scene,
            std::string_view jsonpath)
{
  if (auto _i = libvrm::JsonPath(jsonpath).GetLastInt()) {
    auto i = *_i;
    return [scene, &prims = scene->m_gltf.Meshes[i].Primitives]() {
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
          auto& attributes = prim.Attributes;
          int POSITION = *attributes.POSITION;
          ImGui::Text("%d", scene->m_gltf.Accessors[POSITION].Count);
          ImGui::TableSetColumnIndex(2);
          // std::stringstream ss;
          // for (auto kv : attributes.items()) {
          //   ss << "," << kv.key();
          // }
          // ImGui::Text("%s", ss.str().c_str());
          ImGui::TableSetColumnIndex(3);
          int indices = *prim.Indices;
          ImGui::Text("%d", scene->m_gltf.Accessors[indices].Count);
        }
        ImGui::EndTable();
      }
    };
  }
  return []() {};
}
