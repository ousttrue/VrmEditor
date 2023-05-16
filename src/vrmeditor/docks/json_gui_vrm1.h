#include "json_gui.h"
#include "json_gui_table.h"
#include <imgui.h>
#include <vrm/dmath.h>

inline ShowGui
JsonGuiVrm1SpringColliders(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                           std::string_view jsonpath)
{
  // auto& items = scene->m_gltf.Json.at(nlohmann::json::json_pointer(
  //   std::string{ jsonpath.begin(), jsonpath.end() }));
  // return [items]() {
  //   ImGui::Text("colliders[%zu]", items.size());
  //   std::array<const char*, 6> cols = {
  //     "index", "node", "shape", "offset", "radius", "tail",
  //   };
  //   if (JsonGuiTable("##spring.colliders", cols)) {
  //     for (int i = 0; i < items.size(); ++i) {
  //       auto& item = items[i];
  //       ImGui::TableNextRow();
  //       ImGui::TableSetColumnIndex(0);
  //       ImGui::Text("%d", i);
  //
  //       ImGui::TableSetColumnIndex(1);
  //       ImGui::Text("%d", (int)item.at("node"));
  //
  //       auto& shape = item.at("shape");
  //       if (libvrm::gltf::has(shape, "sphere")) {
  //         ImGui::TableSetColumnIndex(2);
  //         ImGui::TextUnformatted("sphere");
  //         auto& sphere = shape.at("sphere");
  //
  //         ImGui::TableSetColumnIndex(3);
  //         std::stringstream ss;
  //         ss << sphere.value("offset", DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f
  //         }); ImGui::Text("%s", ss.str().c_str());
  //
  //         ImGui::TableSetColumnIndex(4);
  //         ImGui::Text("%f", sphere.value("radius", 0.0f));
  //
  //       } else if (libvrm::gltf::has(shape, "capsule")) {
  //         ImGui::TableSetColumnIndex(2);
  //         ImGui::TextUnformatted("capsule");
  //         auto& capsule = shape.at("capsule");
  //
  //         ImGui::TableSetColumnIndex(3);
  //         std::stringstream ss;
  //         ss << capsule.value("offset", DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f
  //         }); ImGui::Text("%s", ss.str().c_str());
  //
  //         ImGui::TableSetColumnIndex(4);
  //         ImGui::Text("%f", capsule.value("radius", 0.0f));
  //
  //         ImGui::TableSetColumnIndex(5);
  //         ss.str("");
  //         ss << capsule.value("tail", DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f });
  //         ImGui::Text("%s", ss.str().c_str());
  //       } else {
  //         ImGui::TextUnformatted("unknown");
  //       }
  //     }
  //     ImGui::EndTable();
  //   }
  // };
  return [](){};
}

inline ShowGui
JsonGuiVrm1SpringJoints(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                        std::string_view jsonpath)
{
  // auto& items = scene->m_gltf.Json.at(nlohmann::json::json_pointer(
  //   std::string{ jsonpath.begin(), jsonpath.end() }));
  // return [items]() {
  //   ImGui::Text("joints[%zu]", items.size());
  //   std::array<const char*, 5> cols = {
  //     "index", "node", "dragForce", "hitRadius", "stiffness",
  //   };
  //   if (JsonGuiTable("##spring.joints", cols)) {
  //     for (int i = 0; i < items.size(); ++i) {
  //       auto& item = items[i];
  //       ImGui::TableNextRow();
  //       ImGui::TableSetColumnIndex(0);
  //       ImGui::Text("%d", i);
  //
  //       ImGui::TableSetColumnIndex(1);
  //       ImGui::Text("%d", (int)item.at("node"));
  //
  //       ImGui::TableSetColumnIndex(2);
  //       ImGui::Text("%f", item.value("dragForce", 0.0f));
  //
  //       ImGui::TableSetColumnIndex(3);
  //       ImGui::Text("%f", item.value("hitRadius", 0.0f));
  //
  //       ImGui::TableSetColumnIndex(4);
  //       ImGui::Text("%f", item.value("stiffness", 0.0f));
  //     }
  //     ImGui::EndTable();
  //   }
  // };
  return [](){};
}
