#pragma once
#include "json_gui.h"
#include "showgui.h"
#include <grapho/imgui/widgets.h>

// inline ShowGuiFunc
// JsonGuiVrm0ColliderList(const std::shared_ptr<libvrm::gltf::Scene>& scene,
//                         std::string_view jsonpath)
// {
//   std::string path{ jsonpath.begin(), jsonpath.end() };
//   // auto colliders =
//   scene->m_gltf.Json.at(nlohmann::json::json_pointer(path));
//   // return [colliders]() {
//   //   std::array<const char*, 3> cols = { "index", "offet", "radius" };
//   //   std::string no_name;
//   //   if (grapho::imgui::BeginTableColumns("##colliders", cols)) {
//   //     for (int i = 0; i < colliders.size(); ++i) {
//   //       auto& collider = colliders[i];
//   //       ImGui::TableNextRow();
//   //       ImGui::TableSetColumnIndex(0);
//   //       ImGui::Text("%d", i);
//   //
//   //       ImGui::TableSetColumnIndex(1);
//   //       auto& offset = collider.at("offset");
//   //       float x = offset.at("x");
//   //       float y = offset.at("y");
//   //       float z = offset.at("z");
//   //       ImGui::Text("{%f, %f, %f}", x, y, z);
//   //
//   //       ImGui::TableSetColumnIndex(2);
//   //       ImGui::Text("%f", collider.value("radius", 0.0f));
//   //     }
//   //     ImGui::EndTable();
//   //   }
//   // };
//   return []() {};
// }

inline ShowGuiFunc
JsonGuiVrm0SpringList(std::u8string_view jsonpath)
{
  // std::string path{ jsonpath.begin(), jsonpath.end() };
  // auto springs = scene->m_gltf.Json.at(nlohmann::json::json_pointer(path));
  return [](const gltfjson::typing::Root& root,
            const gltfjson::typing::Bin& bin,
            const gltfjson::tree::NodePtr& node) {
    std::array<const char*, 9> cols = {
      "index",      "comment", "dragForce", "gravity",   "radius",
      "stiffiness", "bones",   "center",    "colliders",
    };
    std::string no_name;

    if (auto springs = node->Array()) {
      if (grapho::imgui::BeginTableColumns("##springs", cols)) {
        for (int i = 0; i < springs->size(); ++i) {
          auto& spring = (*springs)[i];
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::Text("%d", i);
          // ImGui::TableSetColumnIndex(1);
          // ImGui::Text("%s", spring->U8String("comment", no_name).c_str());
          // ImGui::TableSetColumnIndex(2);
          // ImGui::Text("%f", spring.value("dragForce", 0.0f));
          // ImGui::TableSetColumnIndex(4);
          // ImGui::Text("%f", spring.value("hitRadius", 0.0f));
          // ImGui::TableSetColumnIndex(5);
          // ImGui::Text("%f", spring.value("stiffiness", 0.0f));
        }
        ImGui::EndTable();
      }
    }
    return false;
  };
}
