#pragma once
#include "json_gui.h"
#include <vrm/node.h>

inline ShowGui
JsonGuiNodeList(const std::shared_ptr<libvrm::gltf::Scene>& scene,
                std::string_view jsonpath)
{
  return [scene]() {
    auto nodes = scene->m_gltf.Json.at("nodes");
    std::array<const char*, 7> cols = {
      "index", "name", "T", "R", "S", "mesh", "skin",
    };
    std::string no_name;
    if (JsonGuiTable("##nodes", cols)) {
      for (int i = 0; i < nodes.size(); ++i) {
        auto& node = nodes[i];
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%d", i);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", node.value("name", no_name).c_str());
        if (has(node, "translation")) {
          auto t = node.value("translation", DirectX::XMFLOAT3{ 0, 0, 0 });
          ImGui::TableSetColumnIndex(2);
          ImGui::Text("%f, %f, %f", t.x, t.y, t.z);
        }
        // ImGui::TableSetColumnIndex(3);
        // ImGui::Text("%f, %f, %f, %f",
        //             node->Transform.Rotation.x,
        //             node->Transform.Rotation.y,
        //             node->Transform.Rotation.z,
        //             node->Transform.Rotation.w);
        // ImGui::TableSetColumnIndex(4);
        // ImGui::Text(
        //   "%f, %f, %f", node->Scaling.x, node->Scaling.y, node->Scaling.z);
        if (has(node, "mesh")) {
          ImGui::TableSetColumnIndex(5);
          ImGui::Text("%d", (int)node.at("mesh"));
        }
        if (has(node, "skin")) {
          ImGui::TableSetColumnIndex(6);
          ImGui::Text("%d", (int)node.at("skin"));
        }
      }
      ImGui::EndTable();
    }
  };
}
